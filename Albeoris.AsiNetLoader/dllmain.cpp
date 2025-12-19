#include <iostream>
#include <thread>
#include <windows.h>
#include <filesystem>
#include <memory>
#include <vector>
#include <TlHelp32.h>

import Albeoris.DotNetRuntimeHost;
import Albeoris.AsiNetLoader.Logging;

using namespace Albeoris::DotNetRuntimeHost;
using namespace Albeoris::AsiNetLoader::Logging;

// Global logger instance
std::shared_ptr<ILogger> g_logger;

// Global list of suspended threads
std::vector<HANDLE> g_suspendedThreads;

void WriteDebugMessage(const char* text)
{
    if (g_logger)
    {
        g_logger->WriteLine(text);
        g_logger->Flush();
    }
}

void WriteDebugMessage(const std::string& text)
{
    WriteDebugMessage(text.c_str());
}

/// <summary>
/// Suspends all threads in the current process except the current thread and the specified thread
/// </summary>
/// <param name="excludeThread">Thread handle to exclude from suspension</param>
void SuspendAllOtherThreads(HANDLE excludeThread)
{
    DWORD currentProcessId = GetCurrentProcessId();
    DWORD currentThreadId = GetCurrentThreadId();
    DWORD excludeThreadId = GetThreadId(excludeThread);
    
    WriteDebugMessage("SuspendAllOtherThreads: Suspending all threads except current and initialization thread");
    
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    if (snapshot == INVALID_HANDLE_VALUE)
    {
        WriteDebugMessage("SuspendAllOtherThreads: Failed to create thread snapshot");
        return;
    }
    
    THREADENTRY32 threadEntry = {};
    threadEntry.dwSize = sizeof(THREADENTRY32);
    
    if (Thread32First(snapshot, &threadEntry))
    {
        do
        {
            // Skip threads not in our process
            if (threadEntry.th32OwnerProcessID != currentProcessId)
                continue;
            
            // Skip current thread and excluded thread
            if (threadEntry.th32ThreadID == currentThreadId || threadEntry.th32ThreadID == excludeThreadId)
                continue;
            
            // Open thread with suspend/resume access
            HANDLE thread = OpenThread(THREAD_SUSPEND_RESUME, FALSE, threadEntry.th32ThreadID);
            if (thread)
            {
                DWORD suspendCount = SuspendThread(thread);
                if (suspendCount != (DWORD)-1)
                {
                    g_suspendedThreads.push_back(thread);
                    WriteDebugMessage("SuspendAllOtherThreads: Suspended thread ID: " + std::to_string(threadEntry.th32ThreadID));
                }
                else
                {
                    CloseHandle(thread);
                }
            }
        } while (Thread32Next(snapshot, &threadEntry));
    }
    
    CloseHandle(snapshot);
    WriteDebugMessage("SuspendAllOtherThreads: Suspended " + std::to_string(g_suspendedThreads.size()) + " threads");
}

/// <summary>
/// Resumes all previously suspended threads
/// </summary>
void ResumeAllSuspendedThreads()
{
    WriteDebugMessage("ResumeAllSuspendedThreads: Resuming " + std::to_string(g_suspendedThreads.size()) + " threads");
    
    for (HANDLE thread : g_suspendedThreads)
    {
        ResumeThread(thread);
        CloseHandle(thread);
    }
    
    g_suspendedThreads.clear();
    WriteDebugMessage("ResumeAllSuspendedThreads: All threads resumed");
}

// Thread function to initialize .NET runtime outside of loader lock
DWORD WINAPI InitializeRuntimeThread(LPVOID lpParam)
{
    HMODULE hModule = reinterpret_cast<HMODULE>(lpParam);
    
    try
    {
        // Get the directory where this DLL is located
        wchar_t dllPath[MAX_PATH];
        GetModuleFileNameW(hModule, dllPath, MAX_PATH);
        std::filesystem::path dllDirectory = std::filesystem::path(dllPath).parent_path();
        
        // Initialize .NET Runtime using HostFactory with absolute path
        std::filesystem::path runtimeConfigPath = dllDirectory / "Albeoris.AsiNetLoader.Managed.runtimeconfig.json";
        
        // Validate that runtime config file exists
        if (!std::filesystem::exists(runtimeConfigPath))
            throw DotNetHostException("Runtime configuration file not found: " + runtimeConfigPath.string());
        
        WriteDebugMessage("InitializeRuntimeThread: Initializing .NET Runtime: " + runtimeConfigPath.string());
        auto host = HostFactory::CreateHost(runtimeConfigPath);
        
        WriteDebugMessage("InitializeRuntimeThread: .NET Runtime initialized successfully");
        
        // Get and log all runtime properties
        auto properties = host->GetRuntimeProperties();
        WriteDebugMessage("InitializeRuntimeThread: Runtime properties count: " + std::to_string(properties.size()));
        for (const auto& [key, value] : properties)
        {
            std::wcout << L"  " << key << L" = " << value << std::endl;
        }
        
        // Get and display the actual runtime version that was loaded
        auto runtimeVersion = host->GetRuntimeVersion();
        WriteDebugMessage("InitializeRuntimeThread: Loaded .NET Runtime version: " + runtimeVersion);
        
        // TODO: Load managed assembly and call methods
        // Example:
        // auto functionPtr = host->LoadAssemblyAndGetFunctionPointer(
        //     L"YourAssembly.dll",
        //     L"YourNamespace.YourClass, YourAssembly",
        //     L"YourMethod"
        // );
    }
    catch (const DotNetHostException& ex)
    {
        WriteDebugMessage("InitializeRuntimeThread: Failed to initialize .NET Runtime");
        WriteDebugMessage(ex.what());
    }
    catch (const std::exception& ex)
    {
        WriteDebugMessage("InitializeRuntimeThread: Unexpected error");
        WriteDebugMessage(ex.what());
    }

    // Resume all suspended threads after CLR initialization
    ResumeAllSuspendedThreads();
    
    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
    // Don't call this method again
    DisableThreadLibraryCalls(hModule);
    
    // std::this_thread::sleep_for(std::chrono::milliseconds(30000));

    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        {
            // Initialize logger first
            wchar_t dllPath[MAX_PATH];
            GetModuleFileNameW(hModule, dllPath, MAX_PATH);
            
            auto loggerFactory = std::make_shared<FileLoggerFactory>(std::filesystem::path(dllPath));
            g_logger = loggerFactory->CreateLogger();
            
            // Log where we're writing to
            if (loggerFactory->IsUsingConsole())
            {
                WriteDebugMessage("DllMain: Using console logger (fallback)");
            }
            else
            {
                WriteDebugMessage("DllMain: Logging to file: " + loggerFactory->GetLogFilePath());
            }
            
            WriteDebugMessage("DllMain: DLL_PROCESS_ATTACH");
            
            // DO NOT initialize .NET runtime here - it causes deadlock due to loader lock!
            // Create a separate thread to initialize the runtime outside of loader lock
            HANDLE hThread = CreateThread(nullptr, 0, InitializeRuntimeThread, hModule, 0, nullptr);
            if (hThread)
            {
                // Suspend all other threads to prevent them from interfering with CLR initialization
                SuspendAllOtherThreads(hThread);
                CloseHandle(hThread);
            }
            else
            {
                WriteDebugMessage("DllMain: Failed to create initialization thread");
            }
        }
        break;
    case DLL_THREAD_ATTACH:
        WriteDebugMessage("DllMain: DLL_THREAD_ATTACH");
        break;
    case DLL_THREAD_DETACH:
        WriteDebugMessage("DllMain: DLL_THREAD_DETACH");
        break;
    case DLL_PROCESS_DETACH:
        WriteDebugMessage("DllMain: DLL_PROCESS_DETACH");
        if (g_logger)
        {
            g_logger->Flush();
        }
        break;
    default:
        break;
    }

    return TRUE;
}

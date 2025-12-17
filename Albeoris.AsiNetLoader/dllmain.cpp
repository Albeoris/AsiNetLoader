#include <iostream>
#include <thread>
#include <windows.h>
#include <filesystem>

import Albeoris.DotNetRuntimeHost;

using namespace Albeoris::DotNetRuntimeHost;

#define ASI_NET_LOADER_WRITE_DEBUG_MESSAGES

void WriteDebugMessage(const char* text)
{
#ifdef ASI_NET_LOADER_WRITE_DEBUG_MESSAGES
    std::cout << text << std::endl;
#endif
}

void WriteDebugMessage(const std::string& text)
{
    WriteDebugMessage(text.c_str());
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
    
    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
    // std::this_thread::sleep_for(std::chrono::milliseconds(30000));

    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        {
            WriteDebugMessage("DllMain: DLL_PROCESS_ATTACH");
            
            // DO NOT initialize .NET runtime here - it causes deadlock due to loader lock!
            // Create a separate thread to initialize the runtime outside of loader lock
            HANDLE hThread = CreateThread(nullptr, 0, InitializeRuntimeThread, hModule, 0, nullptr);
            if (hThread)
            {
                // Close handle because we cannot wait inside DllMain
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
        break;
    default:
        break;
    }

    return TRUE;
}

#include <iostream>
#include <thread>
#include <windows.h>
#include <filesystem>
#include <memory>
#include <vector>
#include <TlHelp32.h>

import DotNetRuntimeHost;
import AsiNetLoader.Logging;

using namespace DotNetRuntimeHost;
using namespace AsiNetLoader::Logging;

// Global logger instance
std::shared_ptr<ILogger> g_logger;

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

// Export InitializeASI function to be called by Ultimate ASI Loader
extern "C" __declspec(dllexport) void InitializeASI()
{
    // Get the handle to this DLL
    HMODULE hModule = nullptr;
    GetModuleHandleExW(
        GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
        reinterpret_cast<LPCWSTR>(&InitializeASI),
        &hModule
    );
    
    if (!hModule)
    {
        return;
    }
    
    // Initialize logger first
    wchar_t dllPath[MAX_PATH];
    GetModuleFileNameW(hModule, dllPath, MAX_PATH);
    
    auto loggerFactory = std::make_shared<FileLoggerFactory>(std::filesystem::path(dllPath));
    g_logger = loggerFactory->CreateLogger();
    
    // Log where we're writing to
    if (loggerFactory->IsUsingConsole())
    {
        WriteDebugMessage("InitializeASI: Using console logger (fallback)");
    }
    else
    {
        WriteDebugMessage("InitializeASI: Logging to file: " + loggerFactory->GetLogFilePath());
    }
    
    WriteDebugMessage("InitializeASI: Starting initialization");
    
    try
    {
        // Get the directory where this DLL is located
        std::filesystem::path dllDirectory = std::filesystem::path(dllPath).parent_path();
        
        // Initialize .NET Runtime using HostFactory with absolute path
        std::filesystem::path runtimeConfigPath = dllDirectory / "NetLoader" / "Runtime" / "AsiNetLoader.Managed.runtimeconfig.json";
        
        // Validate that runtime config file exists
        if (!std::filesystem::exists(runtimeConfigPath))
            throw DotNetHostException("Runtime configuration file not found: " + runtimeConfigPath.string());
        
        WriteDebugMessage("InitializeASI: Initializing .NET Runtime: " + runtimeConfigPath.string());
        auto host = HostFactory::CreateHost(runtimeConfigPath);
        
        WriteDebugMessage("InitializeASI: .NET Runtime initialized successfully");
        
        // Get and display the actual runtime version that was loaded
        auto runtimeVersion = host->GetRuntimeVersion();
        WriteDebugMessage("InitializeASI: Loaded .NET Runtime version: " + runtimeVersion);
        
        // Get and log all runtime properties
        auto properties = host->GetRuntimeProperties();
        WriteDebugMessage("InitializeASI: Runtime properties count: " + std::to_string(properties.size()));
        for (const auto& [key, value] : properties)
        {
            std::wcout << L"  " << key << L" = " << value << std::endl;
        }
        
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
        WriteDebugMessage("InitializeASI: Failed to initialize .NET Runtime");
        WriteDebugMessage(ex.what());
    }
    catch (const std::exception& ex)
    {
        WriteDebugMessage("InitializeASI: Unexpected error");
        WriteDebugMessage(ex.what());
    }
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    // Don't call this method again
    DisableThreadLibraryCalls(hModule);
    
    // DllMain should remain empty - all initialization is done in InitializeASI
    
    return TRUE;
}

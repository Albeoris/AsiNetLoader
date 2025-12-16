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

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
    // std::this_thread::sleep_for(std::chrono::milliseconds(30000));

    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        WriteDebugMessage("DllMain: DLL_PROCESS_ATTACH");
        
        try
        {
            // Initialize .NET Runtime using HostFactory
            std::filesystem::path runtimeConfigPath = "AsiNetLoader.runtimeconfig.json";
            auto host = HostFactory::CreateHost(runtimeConfigPath);
            
            WriteDebugMessage("DllMain: .NET Runtime initialized successfully");
            
            // Get and display the actual runtime version that was loaded
            auto runtimeVersion = host->GetRuntimeVersion();
            WriteDebugMessage("DllMain: Loaded .NET Runtime version: " + runtimeVersion);
            
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
            WriteDebugMessage("DllMain: Failed to initialize .NET Runtime");
            WriteDebugMessage(ex.what());
        }
        catch (const std::exception& ex)
        {
            WriteDebugMessage("DllMain: Unexpected error");
            WriteDebugMessage(ex.what());
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

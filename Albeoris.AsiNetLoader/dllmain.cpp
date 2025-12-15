#include <iostream>
#include <thread>
#include <windows.h>

#define ASI_NET_LOADER_WRITE_DEBUG_MESSAGES

void WriteDebugMessage(const char* text)
{
#ifdef ASI_NET_LOADER_WRITE_DEBUG_MESSAGES
    std::cout << text << std::endl;
#endif
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
    // std::this_thread::sleep_for(std::chrono::milliseconds(30000));

    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        WriteDebugMessage("DllMain: DLL_PROCESS_ATTACH");
        
        // TODO: Demonstrate the new C#-style HostFxr API
        // DemoNewHostFxrAPI();
        
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

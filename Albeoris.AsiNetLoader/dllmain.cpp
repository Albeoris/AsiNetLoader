#include <iostream>
#include <thread>
#include <windows.h>

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
    // std::this_thread::sleep_for(std::chrono::milliseconds(30000));
    std::cout << "HOOKED" << std::endl;
    
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
	
    return TRUE;
}


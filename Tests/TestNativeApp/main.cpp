#include <chrono>
#include <Windows.h>
#include <cstdio>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <thread>

// prototypes from dinput8.dll (import not required for dynamic lookup; kept for signature reference)
extern "C" __declspec(dllimport) HRESULT WINAPI DirectInput8Create(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID* ppvOut, LPUNKNOWN punkOuter);

// Local test function to be hooked later; must be cdecl
extern "C" void __cdecl PrintHello()
{
    #if defined(_WIN64)
    std::puts("Hello from app::PrintHello (cdecl) [x64]");
    #else
    std::puts("Hello from app::PrintHello (cdecl) [x86]");
    #endif
}

std::string GetTimestamp()
{
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

    std::tm tm_now;
    localtime_s(&tm_now, &time_t_now);

    std::ostringstream oss;
    oss << std::put_time(&tm_now, "%Y-%m-%d %H:%M:%S");
    oss << '.' << std::setfill('0') << std::setw(3) << ms.count();
    return oss.str();
}

using DirectInput8Create_t = HRESULT (WINAPI *)(HINSTANCE, DWORD, REFIID, LPVOID*, LPUNKNOWN);

int main()
{
    HMODULE dinput = LoadLibraryW(L"dinput8.dll");
    if (!dinput)
    {
        std::cerr << "[" << GetTimestamp() << "] " << "Failed to load dinput8.dll from " << '\n';
        return 10;
    }

    auto pDirectInput8Create = (DirectInput8Create_t)GetProcAddress(dinput, "DirectInput8Create");
    if (!pDirectInput8Create)
    {
        // x86 stdcall decorated name
        pDirectInput8Create = (DirectInput8Create_t)GetProcAddress(dinput, "_DirectInput8Create@20");
    }

    if (!pDirectInput8Create)
    {
        std::puts("Missing DirectInput8Create export in dinput8.dll");
        return 11;
    }

    LPVOID outObj = nullptr;
    HRESULT hr = pDirectInput8Create(GetModuleHandleW(nullptr), 0x0800, GUID_NULL, &outObj, nullptr);
    std::printf("DirectInput8Create returned: 0x%08lX\n", (unsigned long)hr);

    // std::this_thread::sleep_for(std::chrono::milliseconds(30000));

    // Call local cdecl test function
    PrintHello();

    std::puts(("[" + GetTimestamp() + "]" + "Native app finished.").c_str());
    return 0;
}

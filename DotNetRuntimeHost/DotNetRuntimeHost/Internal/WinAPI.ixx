export module DotNetRuntimeHost:WinAPI;

import :Arguments;
import :DotNetHostException;
import :Types;
import  <format>;
import <Windows.h>;

namespace DotNetRuntimeHost
{
    class WinAPI
    {
    public:
        WinAPI() = delete; // static-only

        static HMODULE LoadLibrary(std::wstring dllName)
        {
            Arguments::ThrowIfNull(&dllName, "dllName");

            const wchar_t* nullTerminatedPath = dllName.c_str();

            HMODULE lib = ::LoadLibraryW(nullTerminatedPath);
            if (!lib)
            {
                const DWORD ec = ::GetLastError();
                throw DotNetHostException(std::format(L"{} could not be loaded (error: {})", dllName, ec));
            }
            return lib;
        }

        template <typename T>
        [[nodiscard]] static T GetProcAddress(HMODULE lib, const char* procName, bool throwOnError = true)
        {
            static_assert(std::is_pointer_v<T> && std::is_function_v<std::remove_pointer_t<T>>,
                          "T must be a function pointer type");

            Arguments::ThrowIfNull(lib, "lib");
            Arguments::ThrowIfNull(procName, "procName");

            FARPROC result = ::GetProcAddress(lib, procName);
            if (!result)
            {
                if (throwOnError)
                {
                    const DWORD ec = ::GetLastError();
                    throw DotNetHostException(std::format("GetProcAddress failed for '{}' (err={})", procName, ec));
                }
                return nullptr;
            }

            return reinterpret_cast<T>(result);
        }

        static std::wstring GetModuleFileName(HMODULE lib)
        {
            Arguments::ThrowIfNull(lib, "lib");

            wchar_t buffer[MAX_PATH];
            DWORD len = ::GetModuleFileNameW(lib, buffer, MAX_PATH);

            if (len == 0)
                throw DotNetHostException("GetModuleFileNameW failed");

            if (len == MAX_PATH)
                throw DotNetHostException("GetModuleFileNameW result is too long");

            return std::wstring(buffer, len);
        }
    };
}

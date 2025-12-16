export module Albeoris.DotNetRuntimeHost:DotNetHostException;

import <stdexcept>;
import <windows.h>;

export namespace Albeoris::DotNetRuntimeHost
{
    struct DotNetHostException : std::runtime_error
    {
        explicit DotNetHostException(const std::string& msg)
            : std::runtime_error(msg)
        {
        }

        explicit DotNetHostException(const std::wstring& msg)
            : std::runtime_error(ToUtf8(msg))
        {
        }

        inline std::string ToUtf8(std::wstring_view str)
        {
            if (str.empty())
                return {};
            
            int len = ::WideCharToMultiByte(CP_UTF8, 0, str.data(), (int)str.size(), nullptr, 0, nullptr, nullptr);

            std::string result(len, '\0');
            ::WideCharToMultiByte(CP_UTF8, 0, str.data(), (int)str.size(), result.data(), len, nullptr, nullptr);

            return result;
        }
    };
}

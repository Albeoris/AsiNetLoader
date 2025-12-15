export module Albeoris.DotNetRuntimeHost:Arguments;

import <format>;
import <string>;
import <stdexcept>;
import <type_traits>;

namespace Albeoris::DotNetRuntimeHost
{
    struct Arguments
    {
        template <typename T>
        static void ThrowIfNull(const T* ptr, std::string_view argumentName)
        {
            if (ptr == nullptr)
                throw std::invalid_argument(std::format("{0} must not be null", argumentName));
        }
    };
}

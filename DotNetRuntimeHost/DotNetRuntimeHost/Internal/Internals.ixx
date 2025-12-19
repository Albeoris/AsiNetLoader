export module DotNetRuntimeHost:Internals;

import :Types;
import <memory>;

namespace DotNetRuntimeHost
{
    template <typename T, typename... Args>
    uptr<T> Instantiate(Args&&... args)
    {
        return std::make_unique<T>(std::forward<Args>(args)...);
    }

    inline constexpr bool IsWindowsBuild =
#ifdef _WIN32
        true;
#else
    false;
#endif
}

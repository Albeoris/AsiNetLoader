export module DotNetRuntimeHost:WindowsHostFactory;

import :IHost;
import :IHostFactory;
import :Internals;
import :Types;
import :WindowsHost;

namespace DotNetRuntimeHost
{
    /// <summary>
    /// Windows-specific implementation of IHostFactory.
    /// </summary>
    class WindowsHostFactory final : public IHostFactory
    {
    public:
        ~WindowsHostFactory() override = default;

        /// <inheritdoc />
        uptr<IHost> CreateHost(const std::filesystem::path& runtimeConfigPath) override
        {
            return Instantiate<WindowsHost>(runtimeConfigPath);
        }
    };
}

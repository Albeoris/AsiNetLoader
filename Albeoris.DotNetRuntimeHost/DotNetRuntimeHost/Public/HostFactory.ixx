export module Albeoris.DotNetRuntimeHost:HostFactory;

import :Exception;
import :IHost;
import :IHostFactory;
import :Internals;
import :Types;
import :WindowsHost;
import :WindowsHostFactory;

export namespace Albeoris::DotNetRuntimeHost
{
    /// <summary>
    /// Platform-specific facade for creating IHost instances.
    /// </summary>
    export class HostFactory final
    {
    public:
        HostFactory() = delete; // static-only

        /// <summary>
        /// Creates an IHost for the current platform.
        /// On Windows: returns a host created by WindowsHostFactory.
        /// On non-Windows: throws Exception.
        /// </summary>
        static uptr<IHost> CreateHost(const std::filesystem::path& runtimeConfigPath)
        {
            if (IsWindowsBuild)
            {
                WindowsHostFactory factory;
                return factory.CreateHost(runtimeConfigPath);
            }
            else
            {
                throw Exception{"HostFactory::CreateHost is only supported on Windows"};
            }
        }
    };
}

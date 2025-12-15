export module Albeoris.DotNetRuntimeHost:IHostFactory;

import <filesystem>;
import <string>;

import :IHost;
import :Types;

namespace Albeoris::DotNetRuntimeHost
{
    /// <summary>
    /// Abstract interface for IHost factory.
    /// </summary>
    class IHostFactory
    {
    public:
        virtual ~IHostFactory() = default;

        /// <summary>
        /// Initializes the .NET runtime host.
        /// </summary>
        virtual uptr<IHost> CreateHost(const std::filesystem::path& runtimeConfigPath) = 0;
    };
}

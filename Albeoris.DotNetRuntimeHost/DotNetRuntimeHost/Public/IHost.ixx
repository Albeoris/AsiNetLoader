export module Albeoris.DotNetRuntimeHost:IHost;

import <filesystem>;
import <string>;

export namespace Albeoris::DotNetRuntimeHost
{
    /// <summary>
    /// Abstract interface for .NET RuntimeHost operations.
    /// </summary>
    export class IHost
    {
    public:
        virtual ~IHost() = default;

        /// <summary>
        /// Loads an assembly and gets a function pointer to a specified method.
        /// </summary>
        /// <param name="assemblyPath">Path to the assembly to load.</param>
        /// <param name="typeName">Name of the type containing the method.</param>
        /// <param name="methodName">Name of the method to get a pointer to.</param>
        /// <returns>Function pointer to the specified method.</returns>
        /// <exception cref="HostFxrException">Thrown when loading fails.</exception>
        virtual void* LoadAssemblyAndGetFunctionPointer(const std::filesystem::path& assemblyPath, const std::wstring& typeName, const std::wstring& methodName) = 0;

        /// <summary>
        /// Gets the actual version of the .NET runtime that has been loaded.
        /// </summary>
        /// <returns>Runtime version string (e.g., "8.0.11").</returns>
        virtual std::string GetRuntimeVersion() = 0;
    };
}

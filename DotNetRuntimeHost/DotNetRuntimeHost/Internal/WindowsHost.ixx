module;

// Include nethost.h from NuGet package Microsoft.NETCore.DotNetAppHost
#include <nethost.h>

export module DotNetRuntimeHost:WindowsHost;

import <cstdint>;
import <filesystem>;
import <format>;
import <iostream>;
import <map>;
import <string>;
import <vector>;
import <Windows.h>;

import :DotNetHostException;
import :HostFxrErrorCodes;
import :IHost;
import :IHostFactory;
import :Types;
import :WinAPI;

namespace DotNetRuntimeHost
{
    // Define function pointer types for the hostfxr functions
    struct HostFxr
    {
        static constexpr int LOAD_ASSEMBLY_AND_GET_FUNCTION_POINTER_DELEGATE_TYPE = 3;
        
        using RuntimeHandle = void*;
        using InitializeForRuntimeConfigDelegate = uint32_t(*)(const wchar_t* runtimeConfigPath, const void* parameters, /*out*/ RuntimeHandle* handle);
        using GetRuntimeDelegate = uint32_t(*)(RuntimeHandle handle, int delegate_type, /*out*/ void** result);
        using GetRuntimePropertyValueDelegate = uint32_t(*)(RuntimeHandle handle, const wchar_t* name, /*out*/ const wchar_t** value);
        using GetRuntimePropertiesDelegate = uint32_t(*)(RuntimeHandle handle, /*out*/ size_t* count, /*out*/ const wchar_t** keys, /*out*/ const wchar_t** values);
        using CloseDelegate = uint32_t(*)(RuntimeHandle handle);
        using LoadAssemblyAndGetFunctionPointerDelegate = uint32_t(*)(const wchar_t* assemblyPath, const wchar_t* typeName, const wchar_t* methodName, const wchar_t* delegateTypeName, void* reserved, /*out*/ void** result);
    };
    
    /// <summary>
    /// Windows-specific implementation of IHost.
    /// </summary>
    class WindowsHost final : public IHost
    {
        friend class WindowsHostFactory;

    private:
        HostFxr::InitializeForRuntimeConfigDelegate _initializeForRuntimeConfig;
        HostFxr::GetRuntimeDelegate _getRuntime;
        HostFxr::GetRuntimePropertyValueDelegate _getRuntimePropertyValue;
        HostFxr::GetRuntimePropertiesDelegate _getRuntimeProperties;
        HostFxr::CloseDelegate _close;

    private:
        /// <summary>
        /// Finds the path to hostfxr.dll using nethost get_hostfxr_path API.
        /// </summary>
        /// <returns>Full path to hostfxr.dll</returns>
        static std::filesystem::path FindHostFxrPath()
        {
            // Use get_hostfxr_path from statically linked nethost.lib
            // get_hostfxr_path is declared in nethost.h
            
            // Get the required buffer size
            size_t buffer_size = 0;
            int rc = get_hostfxr_path(nullptr, &buffer_size, nullptr);
            if (rc != 0 && buffer_size == 0)
            {
                throw DotNetHostException(std::format(L"get_hostfxr_path failed to get buffer size (rc: {})", rc));
            }

            // Allocate buffer and get the path
            std::vector<wchar_t> buffer(buffer_size);
            rc = get_hostfxr_path(buffer.data(), &buffer_size, nullptr);
            
            if (rc != 0)
            {
                throw DotNetHostException(std::format(L"get_hostfxr_path failed (rc: {})", rc));
            }

            return std::filesystem::path(buffer.data());
        }

    public:
        explicit WindowsHost(const std::filesystem::path& runtimeConfigPath)
        {
            HMODULE hostfxrModule = nullptr;
            
            // Check if hostfxr.dll is already loaded in the process
            // This can happen if another .NET runtime version is already initialized
            HMODULE existingHostfxr = GetModuleHandleW(L"hostfxr.dll");
            if (existingHostfxr != nullptr)
            {
                // hostfxr is already loaded, we can use the existing one
                hostfxrModule = existingHostfxr;
            }
            else
            {
                // Find and load hostfxr.dll with full path
                std::filesystem::path hostfxrPath = FindHostFxrPath();
                hostfxrModule = WinAPI::LoadLibrary(hostfxrPath.wstring());
            }
            
            // Load function pointers from hostfxr.dll
            _initializeForRuntimeConfig = WinAPI::GetProcAddress<HostFxr::InitializeForRuntimeConfigDelegate>(hostfxrModule, "hostfxr_initialize_for_runtime_config");
            _getRuntime = WinAPI::GetProcAddress<HostFxr::GetRuntimeDelegate>(hostfxrModule, "hostfxr_get_runtime_delegate");
            _getRuntimePropertyValue = WinAPI::GetProcAddress<HostFxr::GetRuntimePropertyValueDelegate>(hostfxrModule, "hostfxr_get_runtime_property_value");
            _getRuntimeProperties = WinAPI::GetProcAddress<HostFxr::GetRuntimePropertiesDelegate>(hostfxrModule, "hostfxr_get_runtime_properties");
            _close = WinAPI::GetProcAddress<HostFxr::CloseDelegate>(hostfxrModule, "hostfxr_close");

            // Initialize runtime (will handle already initialized runtime gracefully)
            InitializeRuntime(runtimeConfigPath);
        }

        ~WindowsHost() override = default;

    private:
        HostFxr::LoadAssemblyAndGetFunctionPointerDelegate _loadAssemblyAndGetPtr;
        std::string _runtimeVersion;

        /// <summary>
        /// Converts a wide string (UTF-16) to a UTF-8 encoded string.
        /// </summary>
        /// <param name="wstr">The wide string to convert.</param>
        /// <returns>UTF-8 encoded string.</returns>
        static std::string WStringToUTF8(const std::wstring& wstr)
        {
            if (wstr.empty()) return {};
            
            int size = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, nullptr, 0, nullptr, nullptr);
            if (size <= 0) return {};
            
            std::string result(size - 1, 0); // -1 to exclude null terminator
            WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &result[0], size, nullptr, nullptr);
            
            return result;
        }

        /// <summary>
        /// Initializes the .NET runtime with the specified configuration.
        /// </summary>
        /// <param name="runtimeConfigPath">Path to the runtime configuration file.</param>
        /// <param name="hostfxrAlreadyLoaded">True if hostfxr.dll was already loaded in the process.</param>
        /// <exception cref="HostFxrException">Thrown when initialization fails.</exception>
        void InitializeRuntime(const std::filesystem::path& runtimeConfigPath)
        {
            HostFxr::RuntimeHandle hostContext = nullptr;
            uint32_t rc = _initializeForRuntimeConfig(runtimeConfigPath.native().c_str(), nullptr, &hostContext);
            
            // Check if we got an error code indicating incompatible runtime configuration
            if (rc == HostFxrErrorCodes::HostIncompatibleConfig)
            {
                // Runtime is already initialized with a different/incompatible version
                // This means the .runtimeconfig.json requests a framework version that's incompatible
                // with the already loaded runtime in the process.
                
                if (hostContext)
                    _close(hostContext);
                
                throw DotNetHostException(
                    "A .NET runtime is already loaded in this process with an incompatible version. "
                    "The application requires a different version than what's currently running. "
                    "This can happen when:\n"
                    "1. A .NET application has already started and loaded a specific runtime version\n"
                    "2. You're trying to load a component/plugin that requires a different major version\n"
                    "\n"
                    "Solutions:\n"
                    "- Ensure your component targets the same or lower .NET version (e.g., net8.0)\n"
                    "- Configure RollForward=LatestMajor in your .runtimeconfig.json\n"
                    "- Start your component before the main application loads its runtime"
                );
            }
            
            // Check for other errors
            if (rc != HostFxrErrorCodes::Success && rc != HostFxrErrorCodes::Success_HostAlreadyInitialized)
            {
                if (hostContext)
                    _close(hostContext);
                throw DotNetHostException(std::format("hostfxr_initialize_for_runtime_config failed: {}", HostFxrErrorCodes::GetFormattedError(rc)));
            }

            // If we got Success_HostAlreadyInitialized, the runtime was already initialized
            // but we can still use it (it's compatible)
            if (rc == HostFxrErrorCodes::Success_HostAlreadyInitialized)
            {
                _runtimeVersion = "Runtime initialized (reusing existing runtime)";
            }
            else
            {
                _runtimeVersion = "Runtime initialized";
            }

            // Get the delegate for loading assemblies
            void* loadFunc = nullptr;
            rc = _getRuntime(hostContext, HostFxr::LOAD_ASSEMBLY_AND_GET_FUNCTION_POINTER_DELEGATE_TYPE, &loadFunc);
            if (rc != 0 || loadFunc == nullptr)
            {
                _close(hostContext);
                throw DotNetHostException(std::format("hostfxr_get_runtime_delegate failed: {}", HostFxrErrorCodes::GetFormattedError(rc)));
            }

            _loadAssemblyAndGetPtr = reinterpret_cast<HostFxr::LoadAssemblyAndGetFunctionPointerDelegate>(loadFunc);

            _close(hostContext);
        }

    public:
        void* LoadAssemblyAndGetFunctionPointer(const std::filesystem::path& assemblyPath, const std::wstring& typeName, const std::wstring& methodName) override
        {
            // Special marker used by hostfxr when calling load_assembly_and_get_function_pointer.
            // If delegate_type_name is set to (const wchar_t*)-1, it tells the runtime that the target
            // method is attributed with [UnmanagedCallersOnly], so no managed delegate type resolution
            // is needed. In this case hostfxr will return a raw unmanaged function pointer directly.
            auto delegate_type_name = (const wchar_t*)-1;

            void* funcPtr = nullptr;
            uint32_t rc = _loadAssemblyAndGetPtr(
                assemblyPath.wstring().c_str(),
                typeName.c_str(),
                methodName.c_str(),
                delegate_type_name,
                nullptr,
                &funcPtr);

            if (rc != 0 || funcPtr == nullptr)
            {
                throw DotNetHostException(std::format(L"Failed to load assembly or get function pointer for {0} in {1}", methodName, assemblyPath.filename().wstring()));
            }

            return funcPtr;
        }

        std::string GetRuntimeVersion() override
        {
            return _runtimeVersion;
        }

        std::map<std::wstring, std::wstring> GetRuntimeProperties() override
        {
            std::map<std::wstring, std::wstring> properties;

            // First call to get count
            size_t count = 0;
            uint32_t rc = _getRuntimeProperties(nullptr, &count, nullptr, nullptr);
            
            if (rc != HostFxrErrorCodes::HostApiBufferTooSmall || count == 0)
                return properties;

            // Allocate arrays for keys and values
            std::vector<const wchar_t*> keys(count);
            std::vector<const wchar_t*> values(count);

            // Second call to get actual properties
            rc = _getRuntimeProperties(nullptr, &count, keys.data(), values.data());
            
            if (rc != 0)
                throw DotNetHostException(std::format("Failed to get runtime properties: {}", HostFxrErrorCodes::GetFormattedError(rc)));

            // Copy to map
            for (size_t i = 0; i < count; ++i)
            {
                if (keys[i] && values[i])
                {
                    properties[keys[i]] = values[i];
                }
            }

            return properties;
        }
    };
}

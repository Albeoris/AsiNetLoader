export module Albeoris.DotNetRuntimeHost:WindowsHost;

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

namespace Albeoris::DotNetRuntimeHost
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
        /// Finds the path to hostfxr.dll in the .NET installation.
        /// </summary>
        /// <returns>Full path to hostfxr.dll</returns>
        static std::filesystem::path FindHostFxrPath()
        {
            // Try to get DOTNET_ROOT environment variable first
            wchar_t dotnetRoot[MAX_PATH];
            DWORD result = GetEnvironmentVariableW(L"DOTNET_ROOT", dotnetRoot, MAX_PATH);
            
            std::filesystem::path basePath;
            if (result > 0 && result < MAX_PATH)
            {
                basePath = dotnetRoot;
            }
            else
            {
                // Default installation path
                basePath = L"C:\\Program Files\\dotnet";
            }
            
            // Look for the latest version of hostfxr
            std::filesystem::path fxrPath = basePath / "host" / "fxr";
            if (!std::filesystem::exists(fxrPath))
            {
                throw DotNetHostException(std::format(L"Could not find hostfxr directory at: {}", fxrPath.wstring()));
            }
            
            // Find the latest version directory
            std::filesystem::path latestVersion;
            for (const auto& entry : std::filesystem::directory_iterator(fxrPath))
            {
                if (entry.is_directory())
                {
                    if (latestVersion.empty() || entry.path().filename() > latestVersion.filename())
                    {
                        latestVersion = entry.path();
                    }
                }
            }
            
            if (latestVersion.empty())
            {
                throw DotNetHostException(L"No hostfxr version found in .NET installation");
            }
            
            std::filesystem::path hostfxrDll = latestVersion / "hostfxr.dll";
            if (!std::filesystem::exists(hostfxrDll))
            {
                throw DotNetHostException(std::format(L"hostfxr.dll not found at: {}", hostfxrDll.wstring()));
            }
            
            return hostfxrDll;
        }

    public:
        explicit WindowsHost(const std::filesystem::path& runtimeConfigPath)
        {
            // Find and load hostfxr.dll with full path
            std::filesystem::path hostfxrPath = FindHostFxrPath();
            HMODULE lib = WinAPI::LoadLibrary(hostfxrPath.wstring());
            _initializeForRuntimeConfig = WinAPI::GetProcAddress<HostFxr::InitializeForRuntimeConfigDelegate>(lib, "hostfxr_initialize_for_runtime_config");
            _getRuntime = WinAPI::GetProcAddress<HostFxr::GetRuntimeDelegate>(lib, "hostfxr_get_runtime_delegate");
            _getRuntimePropertyValue = WinAPI::GetProcAddress<HostFxr::GetRuntimePropertyValueDelegate>(lib, "hostfxr_get_runtime_property_value");
            _getRuntimeProperties = WinAPI::GetProcAddress<HostFxr::GetRuntimePropertiesDelegate>(lib, "hostfxr_get_runtime_properties");
            _close = WinAPI::GetProcAddress<HostFxr::CloseDelegate>(lib, "hostfxr_close");

            // Initialize runtime
            InitializeRuntime(runtimeConfigPath);
        }

        ~WindowsHost() override = default;

    private:
        bool _runtimeInitialized = false;
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
        /// <exception cref="HostFxrException">Thrown when initialization fails.</exception>
        void InitializeRuntime(const std::filesystem::path& runtimeConfigPath)
        {
            if (_runtimeInitialized)
                return;

            HostFxr::RuntimeHandle hostContext = nullptr;
            uint32_t rc = _initializeForRuntimeConfig(runtimeConfigPath.native().c_str(), nullptr, &hostContext);
            if (rc != 0)
            {
                if (hostContext)
                    _close(hostContext);
                throw DotNetHostException(std::format("hostfxr_initialize_for_runtime_config failed: {}", HostFxrErrorCodes::GetFormattedError(rc)));
            }

            // Commented out - FX_VERSION property doesn't exist in hostfxr_get_runtime_property_value
            // const wchar_t* versionValue = nullptr;
            // rc = _getRuntimePropertyValue(hostContext, L"FX_VERSION", &versionValue);
            // if (rc != 0 || versionValue == nullptr)
            // {
            //     _close(hostContext);
            //     throw DotNetHostException(std::format("Failed to get runtime version property: {}", HostFxrErrorCodes::GetFormattedError(rc)));
            // }
            // _runtimeVersion = WStringToUTF8(versionValue);
            
            _runtimeVersion = "Runtime initialized";

            void* loadFunc = nullptr;
            rc = _getRuntime(hostContext, HostFxr::LOAD_ASSEMBLY_AND_GET_FUNCTION_POINTER_DELEGATE_TYPE, &loadFunc);
            if (rc != 0 || loadFunc == nullptr)
            {
                _close(hostContext);
                throw DotNetHostException(std::format("hostfxr_get_runtime_delegate failed: {}", HostFxrErrorCodes::GetFormattedError(rc)));
            }

            _loadAssemblyAndGetPtr = reinterpret_cast<HostFxr::LoadAssemblyAndGetFunctionPointerDelegate>(loadFunc);

            _close(hostContext);
            _runtimeInitialized = true;
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
            {
                std::cout << "Failed to get runtime properties count or no properties available: " 
                          << HostFxrErrorCodes::GetFormattedError(rc) << std::endl;
                return properties;
            }

            // Allocate arrays for keys and values
            std::vector<const wchar_t*> keys(count);
            std::vector<const wchar_t*> values(count);

            // Second call to get actual properties
            rc = _getRuntimeProperties(nullptr, &count, keys.data(), values.data());
            
            if (rc != 0)
            {
                std::cout << "Failed to get runtime properties: " 
                          << HostFxrErrorCodes::GetFormattedError(rc) << std::endl;
                return properties;
            }

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

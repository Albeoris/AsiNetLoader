export module Albeoris.DotNetRuntimeHost:WindowsHost;

import <filesystem>;
import <string>;
import <Windows.h>;

import :Exception;
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
        using InitializeForRuntimeConfigDelegate = int(*)(const wchar_t* runtimeConfigPath, const void* parameters, /*out*/ RuntimeHandle* handle);
        using GetRuntimeDelegate = int(*)(RuntimeHandle handle, int delegate_type, /*out*/ void** result);
        using CloseDelegate = int(*)(RuntimeHandle handle);
        using LoadAssemblyAndGetFunctionPointerDelegate = int(*)(const wchar_t* assemblyPath, const wchar_t* typeName, const wchar_t* methodName, const wchar_t* delegateTypeName, void* reserved, /*out*/ void** result);
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
        HostFxr::CloseDelegate _close;

    public:
        explicit WindowsHost(const std::filesystem::path& runtimeConfigPath)
        {
            // Load hostfxr.dll
            HMODULE lib = WinAPI::LoadLibrary(L"hostfxr.dll");
            _initializeForRuntimeConfig = WinAPI::GetProcAddress<HostFxr::InitializeForRuntimeConfigDelegate>(lib, "hostfxr_initialize_for_runtime_config");
            _getRuntime = WinAPI::GetProcAddress<HostFxr::GetRuntimeDelegate>(lib, "hostfxr_get_runtime_delegate");
            _close = WinAPI::GetProcAddress<HostFxr::CloseDelegate>(lib, "hostfxr_close");

            // Initialize runtime
            InitializeRuntime(runtimeConfigPath);
        }

        ~WindowsHost() override = default;

    private:
        bool _runtimeInitialized = false;
        HostFxr::LoadAssemblyAndGetFunctionPointerDelegate _loadAssemblyAndGetPtr;

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
            int rc = _initializeForRuntimeConfig(runtimeConfigPath.native().c_str(), nullptr, &hostContext);
            if (rc != 0)
            {
                if (hostContext)
                    _close(hostContext);
                throw Exception(std::format("hostfxr_initialize_for_runtime_config failed with error code: {:#x}", rc));
            }

            void* loadFunc = nullptr;
            rc = _getRuntime(hostContext, HostFxr::LOAD_ASSEMBLY_AND_GET_FUNCTION_POINTER_DELEGATE_TYPE, &loadFunc);
            if (rc != 0 || loadFunc == nullptr)
            {
                _close(hostContext);
                throw Exception(std::format("hostfxr_get_runtime_delegate failed with error code: {:#x}", rc));
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
            int rc = _loadAssemblyAndGetPtr(
                assemblyPath.wstring().c_str(),
                typeName.c_str(),
                methodName.c_str(),
                delegate_type_name,
                nullptr,
                &funcPtr);

            if (rc != 0 || funcPtr == nullptr)
            {
                throw Exception(std::format(L"Failed to load assembly or get function pointer for {0} in {1}", methodName, assemblyPath.filename().wstring()));
            }

            return funcPtr;
        }
    };
}

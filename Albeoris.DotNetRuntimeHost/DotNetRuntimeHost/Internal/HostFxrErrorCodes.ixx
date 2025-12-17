export module Albeoris.DotNetRuntimeHost:HostFxrErrorCodes;

import <cstdint>;
import <string>;
import <format>;

namespace Albeoris::DotNetRuntimeHost
{
    /// <summary>
    /// Provides utilities for working with hostfxr error codes.
    /// Based on: https://github.com/dotnet/runtime/blob/main/docs/design/features/host-error-codes.md
    /// </summary>
    class HostFxrErrorCodes
    {
    public:
        // Success codes
        static constexpr uint32_t Success = 0x00000000;
        static constexpr uint32_t Success_HostAlreadyInitialized = 0x00000001;
        static constexpr uint32_t Success_DifferentRuntimeProperties = 0x00000002;

        // Failure codes
        static constexpr uint32_t InvalidArgFailure = 0x80008081;
        static constexpr uint32_t CoreHostLibLoadFailure = 0x80008082;
        static constexpr uint32_t CoreHostLibMissingFailure = 0x80008083;
        static constexpr uint32_t CoreHostEntryPointFailure = 0x80008084;
        static constexpr uint32_t CoreHostCurHostFindFailure = 0x80008085;
        static constexpr uint32_t CoreClrResolveFailure = 0x80008087;
        static constexpr uint32_t CoreClrBindFailure = 0x80008088;
        static constexpr uint32_t CoreClrInitFailure = 0x80008089;
        static constexpr uint32_t CoreClrExeFailure = 0x8000808a;
        static constexpr uint32_t ResolverInitFailure = 0x8000808b;
        static constexpr uint32_t ResolverResolveFailure = 0x8000808c;
        static constexpr uint32_t LibHostInitFailure = 0x8000808e;
        static constexpr uint32_t LibHostSdkFindFailure = 0x80008091;
        static constexpr uint32_t LibHostInvalidArgs = 0x80008092;
        static constexpr uint32_t InvalidConfigFile = 0x80008093;
        static constexpr uint32_t AppArgNotRunnable = 0x80008094;
        static constexpr uint32_t AppHostExeNotBoundFailure = 0x80008095;
        static constexpr uint32_t FrameworkMissingFailure = 0x80008096;
        static constexpr uint32_t HostApiFailed = 0x80008097;
        static constexpr uint32_t HostApiBufferTooSmall = 0x80008098;
        static constexpr uint32_t AppPathFindFailure = 0x8000809a;
        static constexpr uint32_t SdkResolveFailure = 0x8000809b;
        static constexpr uint32_t FrameworkCompatFailure = 0x8000809c;
        static constexpr uint32_t FrameworkCompatRetry = 0x8000809d;
        static constexpr uint32_t BundleExtractionFailure = 0x8000809f;
        static constexpr uint32_t BundleExtractionIOError = 0x800080a0;
        static constexpr uint32_t LibHostDuplicateProperty = 0x800080a1;
        static constexpr uint32_t HostApiUnsupportedVersion = 0x800080a2;
        static constexpr uint32_t HostInvalidState = 0x800080a3;
        static constexpr uint32_t HostPropertyNotFound = 0x800080a4;
        static constexpr uint32_t HostIncompatibleConfig = 0x800080a5;
        static constexpr uint32_t HostApiUnsupportedScenario = 0x800080a6;
        static constexpr uint32_t HostFeatureDisabled = 0x800080a7;

        /// <summary>
        /// Gets a human-readable description of a hostfxr error code.
        /// </summary>
        /// <param name="errorCode">The error code returned by hostfxr functions.</param>
        /// <returns>A string describing the error.</returns>
        static std::string GetErrorDescription(uint32_t errorCode)
        {
            switch (errorCode)
            {
                case Success:
                    return "Success: Operation was successful";
                
                case Success_HostAlreadyInitialized:
                    return "Success_HostAlreadyInitialized: Initialization was successful, but another host context is already initialized";
                
                case Success_DifferentRuntimeProperties:
                    return "Success_DifferentRuntimeProperties: Initialization was successful, but another host context is already initialized with different runtime properties";
                
                case InvalidArgFailure:
                    return "InvalidArgFailure: One or more arguments are invalid";
                
                case CoreHostLibLoadFailure:
                    return "CoreHostLibLoadFailure: Failed to load a hosting component (missing dependencies, corrupt install)";
                
                case CoreHostLibMissingFailure:
                    return "CoreHostLibMissingFailure: One of the hosting components is missing (hostfxr, hostpolicy or coreclr)";
                
                case CoreHostEntryPointFailure:
                    return "CoreHostEntryPointFailure: One of the hosting components is missing a required entry point";
                
                case CoreHostCurHostFindFailure:
                    return "CoreHostCurHostFindFailure: Failed to determine the .NET installation location";
                
                case CoreClrResolveFailure:
                    return "CoreClrResolveFailure: The coreclr library could not be found";
                
                case CoreClrBindFailure:
                    return "CoreClrBindFailure: Failed to load the coreclr library or finding one of the required entry points";
                
                case CoreClrInitFailure:
                    return "CoreClrInitFailure: Call to coreclr_initialize failed";
                
                case CoreClrExeFailure:
                    return "CoreClrExeFailure: Call to coreclr_execute_assembly failed";
                
                case ResolverInitFailure:
                    return "ResolverInitFailure: Initialization of the hostpolicy dependency resolver failed (missing or invalid .deps.json)";
                
                case ResolverResolveFailure:
                    return "ResolverResolveFailure: Resolution of dependencies in hostpolicy failed";
                
                case LibHostInitFailure:
                    return "LibHostInitFailure: Initialization of the hostpolicy library failed (version mismatch)";
                
                case LibHostSdkFindFailure:
                    return "LibHostSdkFindFailure: Failed to find the requested SDK";
                
                case LibHostInvalidArgs:
                    return "LibHostInvalidArgs: Arguments to hostpolicy are invalid";
                
                case InvalidConfigFile:
                    return "InvalidConfigFile: The .runtimeconfig.json file is invalid or missing";
                
                case AppArgNotRunnable:
                    return "AppArgNotRunnable: Command line for dotnet.exe doesn't contain path to the application to run (internal error)";
                
                case AppHostExeNotBoundFailure:
                    return "AppHostExeNotBoundFailure: Apphost failed to determine which application to run";
                
                case FrameworkMissingFailure:
                    return "FrameworkMissingFailure: Failed to find a compatible framework version";
                
                case HostApiFailed:
                    return "HostApiFailed: Host command failed";
                
                case HostApiBufferTooSmall:
                    return "HostApiBufferTooSmall: Buffer provided to a host API is too small";
                
                case AppPathFindFailure:
                    return "AppPathFindFailure: Application path imprinted in apphost doesn't exist";
                
                case SdkResolveFailure:
                    return "SdkResolveFailure: Failed to find the requested SDK";
                
                case FrameworkCompatFailure:
                    return "FrameworkCompatFailure: Application has multiple incompatible references to the same framework";
                
                case FrameworkCompatRetry:
                    return "FrameworkCompatRetry: Internal error in framework resolution algorithm";
                
                case BundleExtractionFailure:
                    return "BundleExtractionFailure: Error extracting single-file bundle (corrupted bundle)";
                
                case BundleExtractionIOError:
                    return "BundleExtractionIOError: Error reading or writing files during single-file bundle extraction";
                
                case LibHostDuplicateProperty:
                    return "LibHostDuplicateProperty: The .runtimeconfig.json contains a runtime property produced by the hosting layer";
                
                case HostApiUnsupportedVersion:
                    return "HostApiUnsupportedVersion: Feature requires a newer version of the hosting layer";
                
                case HostInvalidState:
                    return "HostInvalidState: Current state is incompatible with the requested operation";
                
                case HostPropertyNotFound:
                    return "HostPropertyNotFound: Property requested doesn't exist";
                
                case HostIncompatibleConfig:
                    return "HostIncompatibleConfig: Host configuration is incompatible with existing host context";
                
                case HostApiUnsupportedScenario:
                    return "HostApiUnsupportedScenario: Hosting API does not support the requested scenario";
                
                case HostFeatureDisabled:
                    return "HostFeatureDisabled: Support for the requested feature is disabled";
                
                default:
                    return std::format("Unknown error code: {:#x}", errorCode);
            }
        }

        /// <summary>
        /// Gets a formatted error message with both code and description.
        /// </summary>
        /// <param name="errorCode">The error code returned by hostfxr functions.</param>
        /// <returns>A formatted string with error code and description.</returns>
        static std::string GetFormattedError(uint32_t errorCode)
        {
            return std::format("{:#x}: {}", errorCode, GetErrorDescription(errorCode));
        }
    };
}

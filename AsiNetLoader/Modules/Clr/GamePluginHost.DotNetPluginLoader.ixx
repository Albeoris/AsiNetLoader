export module GamePluginHost:DotNetPluginLoader;
import :Platform;
import DotNetRuntimeHost;
import <vector>;
import <filesystem>;
import <cassert>;
import <string>;
import <format>;
import <memory>;

using namespace std::literals;
using namespace DotNetRuntimeHost;

namespace GamePluginHost
{
    /// <summary>
    /// Main class responsible for hosting the .NET runtime and loading managed plugins.
    /// </summary>
    /// <remarks>
    /// DotNetHost provides methods to initialize the .NET CLR (CoreCLR) and load managed assemblies from a plugin directory.
    /// It uses the HostFxr interface to start .NET and get function pointers to managed methods. 
    /// All methods in this class are static because only one .NET runtime can exist per process.
    /// </remarks>
    export class DotNetPluginLoader final
    {
    private:
        std::unique_ptr<IHost> _hostFxr = nullptr;
        
    public:
        /// <summary>
        /// Initializes the .NET runtime using the specified runtime configuration, if not already initialized.
        /// </summary>
        /// <param name="runtimeConfigPath">
        /// Path to the <c>.runtimeconfig.json</c> file that specifies runtime settings (framework version, etc.).
        /// Typically, this is <c>./Plugins/Net/runtimeconfig.json</c>.
        /// </param>
        /// <returns>
        /// <c>true</c> if the runtime was successfully initialized or was already initialized; <c>false</c> if an error occurred.
        /// </returns>
        explicit DotNetPluginLoader(const std::filesystem::path& runtimeConfigPath)
        {
            _hostFxr = HostFactory::CreateHost(runtimeConfigPath);
        }

        /// <summary>
        /// Loads all managed plugin assemblies from the specified directory and calls their entry point method.
        /// </summary>
        /// <param name="pluginDirectory">
        /// Directory path containing managed plugin assemblies (.dll files) to load.
        /// For example, <c>./Plugins/Net</c>.
        /// </param>
        /// <returns>
        /// <c>true</c> if all plugins were loaded and initialized without critical errors.
        /// Returns <c>false</c> if one or more plugins failed to load or initialize (errors will be logged).
        /// </returns>
        bool LoadPlugins(const std::filesystem::path& pluginDirectory)
        {
            //Logger::Log(L"Scanning for managed plugin assemblies in: " + pluginDirectory.wstring());
            bool allSuccess = true;
            std::error_code ec;
            
            if (!std::filesystem::is_directory(pluginDirectory, ec))
            {
                //Logger::Log(L"Error: Plugin directory not found: " + pluginDirectory.wstring());
                return false;
            }

            for (const auto& file : std::filesystem::directory_iterator(pluginDirectory))
            {
                if (!file.is_regular_file()) continue;
                auto path = file.path();
                if (path.extension() != L".dll") continue; // Only consider .dll files

                //Logger::Log(L"Loading plugin assembly: " + path.wstring());
                if (!InitializePlugin(path))
                {
                    //Logger::Log(L"Failed to initialize plugin: " + path.filename().wstring());
                    allSuccess = false;
                    // Continue loading other plugins despite failure.
                }
            }

            //Logger::Log(L"Managed plugin loading completed. Success = " + std::wstring(allSuccess ? L"true" : L"false"));
            return allSuccess;
        }

    private:
        /// <summary>
        /// Uses HostFxr to load and initialize a managed assembly, then calls its EntryPoint.Initialize method.
        /// </summary>
        /// <param name="assemblyPath">Filesystem path to the managed assembly (.dll) to load.</param>
        /// <returns><c>true</c> if the assembly was loaded and its initialize method executed successfully; <c>false</c> otherwise.</returns>
        bool InitializePlugin(const std::filesystem::path& assemblyPath)
        {
            try
            {
                void* initializeFunctionPtr = _hostFxr->LoadAssemblyAndGetFunctionPointer(assemblyPath, L"EntryPoint"s, L"Initialize"s);
                
                // Cast the function pointer to the appropriate signature: void function(void).
                using EntryPointInitializeDelegate = void(*)();
                EntryPointInitializeDelegate initFunc = reinterpret_cast<EntryPointInitializeDelegate>(initializeFunctionPtr);
                
                //Logger::Log(L"Invoking EntryPoint.Initialize for " + assemblyPath.filename().wstring());
                initFunc();
                return true;
            }
            catch (const DotNetHostException& ex)
            {
                std::string message = ex.what();
                std::wstring wmessage(message.begin(), message.end());
                //Logger::Log(L"HostFxr error loading assembly " + assemblyPath.filename().wstring() + L": " + wmessage);
                return false;
            }
            catch (const std::exception& ex)
            {
                std::string message = ex.what();
                std::wstring wmessage(message.begin(), message.end());
                //Logger::Log(L"Exception in managed initialization for " + assemblyPath.filename().wstring() + L": " + wmessage);
                return false;
            }
            catch (...)
            {
                //Logger::Log(L"Unknown exception in managed initialization for " + assemblyPath.filename().wstring());
                return false;
            }
        }
    };
} // namespace GamePluginHost

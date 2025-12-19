using System.Runtime.InteropServices;

namespace AsiNetLoader.Managed;

/// <summary>
/// Bootstrap class for the managed plugin loader.
/// This is the entry point called from native code.
/// </summary>
public static class Bootstrap
{
    private static PluginLoader? _pluginLoader;
    private static PluginContext? _pluginContext;

    /// <summary>
    /// Initializes the managed plugin system.
    /// Called from native ASI loader.
    /// </summary>
    [UnmanagedCallersOnly]
    public static void Initialize()
    {
        try
        {
            var logger = new ConsoleLogger();
            logger.Info("Bootstrap.Initialize() called");
            logger.Info($"Runtime: {Environment.Version}");
            logger.Info($"Platform: {Environment.OSVersion.Platform}");
            logger.Info($"Is64BitProcess: {Environment.Is64BitProcess}");

            // Get plugin directory (relative to AsiNetLoader.dll location)
            var loaderPath = Path.GetDirectoryName(typeof(Bootstrap).Assembly.Location);
            if (loaderPath == null)
            {
                logger.Error("Failed to determine loader directory");
                return;
            }

            // Plugins are in Output/Plugin/x64 or x86, managed plugins in Output/Plugin/x64/Managed/<PluginName>
            var pluginsDirectory = loaderPath;

            _pluginContext = new PluginContext(logger, pluginsDirectory);
            _pluginLoader = new PluginLoader(logger);

            var plugins = _pluginLoader.LoadPlugins(pluginsDirectory, _pluginContext);
            
            foreach (var plugin in plugins)
            {
                _pluginContext.AddPlugin(plugin);
            }

            logger.Info($"Successfully loaded {plugins.Count} plugin(s)");
        }
        catch (Exception ex)
        {
            Console.WriteLine($"[ERROR] Bootstrap initialization failed: {ex}");
        }
    }

    /// <summary>
    /// Shutdown the plugin system.
    /// Called when application exits.
    /// </summary>
    [UnmanagedCallersOnly]
    public static void Shutdown()
    {
        try
        {
            _pluginLoader?.UnloadAll();
            Console.WriteLine("[INFO] Bootstrap.Shutdown() completed");
        }
        catch (Exception ex)
        {
            Console.WriteLine($"[ERROR] Bootstrap shutdown failed: {ex}");
        }
    }
}

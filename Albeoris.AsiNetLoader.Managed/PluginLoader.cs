using System.Reflection;
using System.Runtime.Loader;
using Albeoris.AsiNetLoader.Contracts;

namespace Albeoris.AsiNetLoader.Managed;

internal class PluginLoader
{
    private readonly IPluginLogger _logger;
    private readonly List<(AssemblyLoadContext Context, IPlugin Plugin)> _loadedPlugins = new();

    public PluginLoader(IPluginLogger logger)
    {
        _logger = logger;
    }

    public IReadOnlyList<IPlugin> LoadPlugins(string pluginsDirectory, IPluginContext context)
    {
        _logger.Info($"Loading plugins from: {pluginsDirectory}");

        if (!Directory.Exists(pluginsDirectory))
        {
            _logger.Warning($"Plugins directory not found: {pluginsDirectory}");
            return Array.Empty<IPlugin>();
        }

        var managedDir = Path.Combine(pluginsDirectory, "Managed");
        if (!Directory.Exists(managedDir))
        {
            _logger.Warning($"Managed plugins directory not found: {managedDir}");
            return Array.Empty<IPlugin>();
        }

        var pluginDirs = Directory.GetDirectories(managedDir);
        _logger.Info($"Found {pluginDirs.Length} plugin directories");

        var plugins = new List<IPlugin>();

        foreach (var pluginDir in pluginDirs)
        {
            try
            {
                var loadedPlugins = LoadPluginFromDirectory(pluginDir, context);
                plugins.AddRange(loadedPlugins);
            }
            catch (Exception ex)
            {
                _logger.Error($"Failed to load plugin from {pluginDir}: {ex.Message}");
            }
        }

        return plugins;
    }

    private List<IPlugin> LoadPluginFromDirectory(string pluginDir, IPluginContext context)
    {
        var pluginName = Path.GetFileName(pluginDir);
        _logger.Info($"Loading plugin: {pluginName}");

        var dllFiles = Directory.GetFiles(pluginDir, "*.dll");
        if (dllFiles.Length == 0)
        {
            _logger.Warning($"No DLL files found in {pluginDir}");
            return new List<IPlugin>();
        }

        var plugins = new List<IPlugin>();

        // Create isolated AssemblyLoadContext for this plugin
        var alc = new PluginLoadContext(pluginName, pluginDir);

        foreach (var dllFile in dllFiles)
        {
            try
            {
                var assembly = alc.LoadFromAssemblyPath(dllFile);
                var pluginTypes = FindPluginTypes(assembly);

                foreach (var pluginType in pluginTypes)
                {
                    var plugin = CreatePluginInstance(pluginType, context);
                    if (plugin != null)
                    {
                        plugins.Add(plugin);
                        _loadedPlugins.Add((alc, plugin));
                        _logger.Info($"Loaded plugin: {plugin.Name} v{plugin.Version} from {Path.GetFileName(dllFile)}");
                    }
                }
            }
            catch (Exception ex)
            {
                _logger.Error($"Failed to load assembly {Path.GetFileName(dllFile)}: {ex.Message}");
            }
        }

        return plugins;
    }

    private static IEnumerable<Type> FindPluginTypes(Assembly assembly)
    {
        return assembly.GetExportedTypes()
            .Where(t => typeof(IPlugin).IsAssignableFrom(t) && 
                       !t.IsInterface && 
                       !t.IsAbstract);
    }

    private IPlugin? CreatePluginInstance(Type pluginType, IPluginContext context)
    {
        try
        {
            var plugin = (IPlugin?)Activator.CreateInstance(pluginType);
            if (plugin == null)
            {
                _logger.Error($"Failed to create instance of {pluginType.FullName}");
                return null;
            }

            plugin.Initialize(context);
            return plugin;
        }
        catch (Exception ex)
        {
            _logger.Error($"Failed to initialize plugin {pluginType.FullName}: {ex.Message}");
            return null;
        }
    }

    public void UnloadAll()
    {
        _logger.Info("Unloading all plugins...");

        foreach (var (alc, plugin) in _loadedPlugins)
        {
            try
            {
                plugin.Shutdown();
                _logger.Info($"Shut down plugin: {plugin.Name}");
            }
            catch (Exception ex)
            {
                _logger.Error($"Error during plugin shutdown: {ex.Message}");
            }
        }

        _loadedPlugins.Clear();
    }
}

/// <summary>
/// Custom AssemblyLoadContext for plugin isolation.
/// Loads plugin dependencies from plugin directory.
/// </summary>
internal class PluginLoadContext : AssemblyLoadContext
{
    private readonly AssemblyDependencyResolver _resolver;

    public PluginLoadContext(string name, string pluginPath) : base(name, isCollectible: true)
    {
        _resolver = new AssemblyDependencyResolver(pluginPath);
    }

    protected override Assembly? Load(AssemblyName assemblyName)
    {
        // Let shared dependencies (Contracts) load from Default context
        if (assemblyName.Name == "Albeoris.AsiNetLoader.Contracts")
            return null;

        var assemblyPath = _resolver.ResolveAssemblyToPath(assemblyName);
        if (assemblyPath != null)
            return LoadFromAssemblyPath(assemblyPath);

        return null;
    }

    protected override IntPtr LoadUnmanagedDll(string unmanagedDllName)
    {
        var libraryPath = _resolver.ResolveUnmanagedDllToPath(unmanagedDllName);
        if (libraryPath != null)
            return LoadUnmanagedDllFromPath(libraryPath);

        return IntPtr.Zero;
    }
}

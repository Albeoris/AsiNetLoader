using AsiNetLoader.Contracts;

namespace AsiNetLoader.Managed;

internal class PluginContext : IPluginContext
{
    private readonly List<IPlugin> _loadedPlugins = new();
    private readonly IPluginLogger _logger;
    private readonly String _pluginDirectory;

    public PluginContext(IPluginLogger logger, String pluginDirectory)
    {
        _logger = logger;
        _pluginDirectory = pluginDirectory;
    }

    public IPluginLogger Logger => _logger;
    public String PluginDirectory => _pluginDirectory;

    public IReadOnlyList<IPlugin> GetLoadedPlugins() => _loadedPlugins.AsReadOnly();

    public IPlugin? FindPlugin(String name) => 
        _loadedPlugins.FirstOrDefault(p => p.Name.Equals(name, StringComparison.OrdinalIgnoreCase));

    internal void AddPlugin(IPlugin plugin) => _loadedPlugins.Add(plugin);
}

using Albeoris.AsiNetLoader.Contracts;

namespace Albeoris.AsiNetLoader.Managed;

internal class PluginContext : IPluginContext
{
    private readonly List<IPlugin> _loadedPlugins = new();
    private readonly IPluginLogger _logger;
    private readonly string _pluginDirectory;

    public PluginContext(IPluginLogger logger, string pluginDirectory)
    {
        _logger = logger;
        _pluginDirectory = pluginDirectory;
    }

    public IPluginLogger Logger => _logger;
    public string PluginDirectory => _pluginDirectory;

    public IReadOnlyList<IPlugin> GetLoadedPlugins() => _loadedPlugins.AsReadOnly();

    public IPlugin? FindPlugin(string name) => 
        _loadedPlugins.FirstOrDefault(p => p.Name.Equals(name, StringComparison.OrdinalIgnoreCase));

    internal void AddPlugin(IPlugin plugin) => _loadedPlugins.Add(plugin);
}

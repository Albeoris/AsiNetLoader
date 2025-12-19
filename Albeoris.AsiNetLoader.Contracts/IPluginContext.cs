namespace Albeoris.AsiNetLoader.Contracts;

/// <summary>
/// Context provided to plugins during initialization.
/// Provides access to loader services and other plugins.
/// </summary>
public interface IPluginContext
{
    /// <summary>
    /// Logger for plugin messages.
    /// </summary>
    IPluginLogger Logger { get; }
    
    /// <summary>
    /// Directory where plugin assemblies are located.
    /// </summary>
    String PluginDirectory { get; }
    
    /// <summary>
    /// Get all loaded plugins.
    /// </summary>
    IReadOnlyList<IPlugin> GetLoadedPlugins();
    
    /// <summary>
    /// Find plugin by name.
    /// </summary>
    IPlugin? FindPlugin(String name);
}

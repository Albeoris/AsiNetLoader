namespace Albeoris.AsiNetLoader.Contracts;

/// <summary>
/// Base interface for all plugins.
/// Must be implemented by plugin entry point class.
/// </summary>
public interface IPlugin
{
    /// <summary>
    /// Plugin name for identification.
    /// </summary>
    String Name { get; }
    
    /// <summary>
    /// Plugin version.
    /// </summary>
    String Version { get; }
    
    /// <summary>
    /// Initialize the plugin.
    /// Called once when plugin is loaded.
    /// </summary>
    /// <param name="context">Plugin context with access to loader services.</param>
    void Initialize(IPluginContext context);
    
    /// <summary>
    /// Shutdown the plugin and cleanup resources.
    /// Called when plugin is being unloaded or application exits.
    /// </summary>
    void Shutdown();
}

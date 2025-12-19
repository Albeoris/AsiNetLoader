namespace Albeoris.AsiNetLoader.Contracts;

/// <summary>
/// Logger interface for plugins.
/// </summary>
public interface IPluginLogger
{
    void Info(String message);
    void Warning(String message);
    void Error(String message);
    void Debug(String message);
}

namespace Albeoris.AsiNetLoader.Contracts;

/// <summary>
/// Logger interface for plugins.
/// </summary>
public interface IPluginLogger
{
    void Info(string message);
    void Warning(string message);
    void Error(string message);
    void Debug(string message);
}

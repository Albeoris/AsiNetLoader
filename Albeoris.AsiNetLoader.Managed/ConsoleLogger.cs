using Albeoris.AsiNetLoader.Contracts;

namespace Albeoris.AsiNetLoader.Managed;

internal class ConsoleLogger : IPluginLogger
{
    public void Info(String message) => 
        Console.WriteLine($"[INFO] {message}");

    public void Warning(String message) => 
        Console.WriteLine($"[WARN] {message}");

    public void Error(String message) => 
        Console.WriteLine($"[ERROR] {message}");

    public void Debug(String message) => 
        Console.WriteLine($"[DEBUG] {message}");
}

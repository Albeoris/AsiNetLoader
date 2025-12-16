namespace Albeoris.AsiNetLoader.Managed;

/// <summary>
/// Bootstrap class for the managed plugin loader.
/// This is the entry point called from native code.
/// </summary>
public static class Bootstrap
{
    /// <summary>
    /// Initializes the managed plugin system.
    /// Called from native ASI loader.
    /// </summary>
    public static void Initialize()
    {
        Console.WriteLine("[AsiNetLoader.Managed] Bootstrap.Initialize() called");
        Console.WriteLine($"[AsiNetLoader.Managed] Runtime: {Environment.Version}");
        Console.WriteLine($"[AsiNetLoader.Managed] Platform: {Environment.OSVersion.Platform}");
        Console.WriteLine($"[AsiNetLoader.Managed] Is64BitProcess: {Environment.Is64BitProcess}");
    }
}

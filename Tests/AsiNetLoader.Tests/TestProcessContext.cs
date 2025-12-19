using System;
using System.Diagnostics;
using System.IO;
using Microsoft.VisualBasic.FileIO;
using Xunit;

namespace AsiNetLoader.Tests;

internal sealed class TestProcessContext(String platform, String exeRelativePath)
{
    public String Platform { get; } = platform;
    public ProcessStartInfo StartInfo { get; } = GetProcessStartInfo(platform, exeRelativePath);
    
    public String StdOut { get; set; }
    public String StdErr { get; set; }
    
    public static TestProcessContext CreateX64Native() => new("x64", "TestNativeAppX64.exe");
    
    public static TestProcessContext Create(String platform, String exeName) => new(platform, exeName);

    private static ProcessStartInfo GetProcessStartInfo(String platform, String exeRelativePath)
    {
        String outputDir = GetOutputFolder();
        String exePath = Path.Combine(outputDir, platform, exeRelativePath);
        Assert.True(File.Exists(exePath), $"Executable not found: {exePath}");

        ProcessStartInfo psi = new ProcessStartInfo
        {
            FileName = exePath,
            WorkingDirectory = Path.GetDirectoryName(exePath)!,
            UseShellExecute = false,
            RedirectStandardOutput = true,
            RedirectStandardError = true,
            CreateNoWindow = true
        };
        return psi;
    }

    public String PluginsDirectoryPath => Path.Combine(StartInfo.WorkingDirectory, "Plugins");

    public String GetPrimaryLogPath() => Path.Combine(PluginsDirectoryPath, "Logs", "AsiNetLoader.log");
    public String GetSecondaryLogPath(String executablePath)
    {
        String executableName = Path.GetFileNameWithoutExtension(executablePath); 
        return Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData), "Albeoris", "AsiNetLoader", executableName, "Logs", "AsiNetLoader.log");
    }

    public String ReadAllOutputText(String executablePath)
    {
        String primaryLogPath = GetPrimaryLogPath();
        if (File.Exists(primaryLogPath))
            return File.ReadAllText(primaryLogPath);
        
        String secondaryLogPath = GetSecondaryLogPath(executablePath);
        if (File.Exists(secondaryLogPath))
            return File.ReadAllText(secondaryLogPath);

        return StdOut;
    }

    private static String GetOutputFolder()
    {
        // Tests project OutDir points to ..\..\Output\Tests\
        // The native apps are expected in subfolders x64/ and x86/ relative to that.
        return AppContext.BaseDirectory.TrimEnd(Path.DirectorySeparatorChar, Path.AltDirectorySeparatorChar);
    }
}
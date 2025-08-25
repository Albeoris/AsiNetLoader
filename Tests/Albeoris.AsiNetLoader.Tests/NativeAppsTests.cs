using System;
using System.Diagnostics;
using System.IO;
using Xunit;

namespace Albeoris.AsiNetLoader.Tests;

public class NativeAppsTests
{
    [Fact(DisplayName = "Run x64 native test app")]
    public void Run_X64_Native_App()
    {
        RunAndAssertExitCodeZero("x64", "Albeoris.TestNativeAppX64.exe");
    }

    [Fact(DisplayName = "Run x86 native test app")]
    public void Run_X86_Native_App()
    {
        RunAndAssertExitCodeZero("x86", "Albeoris.TestNativeAppX86.exe");
    }
    
    [Fact(DisplayName = "Hook x64 native test app")]
    public void Hook_X64_Native_App()
    {
        HookAndAssertExitCodeZero("x64", "Albeoris.TestNativeAppX64.exe");
    }

    [Fact(DisplayName = "Hook x86 native test app")]
    public void Hook_X86_Native_App()
    {
        HookAndAssertExitCodeZero("x86", "Albeoris.TestNativeAppX86.exe");
    }

    private static void RunAndAssertExitCodeZero(String platform, String exeRelativePath)
    {
        ProcessStartInfo psi = GetProcessStartInfo(platform, exeRelativePath);

        using Process proc = Process.Start(psi);
        Assert.NotNull(proc);
        
        String stdout = proc.StandardOutput.ReadToEnd();
        String stderr = proc.StandardError.ReadToEnd();
        proc.WaitForExit(30_000);

        Assert.True(proc.HasExited, "Process did not exit within timeout");
        Assert.True(proc.ExitCode == 0, $"Process exited with code {proc.ExitCode}.\nSTDOUT:\n{stdout}\nSTDERR:\n{stderr}");
        Assert.True(stdout.Contains($"Hello from app::PrintHello (cdecl) [{platform}]"), $"Process did not print hello message.\nSTDOUT:\n{stdout}\nSTDERR:\n{stderr}");
    }
    
    private static void HookAndAssertExitCodeZero(String platform, String exeRelativePath)
    {
        ProcessStartInfo psi = GetProcessStartInfo(platform, exeRelativePath);
        String workingDirectory = Path.GetDirectoryName(psi.FileName);
        Assert.NotNull(workingDirectory);

        String pluginsFolder = Path.Combine(workingDirectory, "Plugins");
        Directory.CreateDirectory(pluginsFolder);
        
        String targetPluginPath = Path.Combine(pluginsFolder, $"Albeoris.AsiNetLoader{platform}.asi");
        String sourcePluginPath = Path.GetFullPath(Path.Combine(workingDirectory, "..", "..", "Plugin", platform, $"Albeoris.AsiNetLoader{platform}.asi"));
        Assert.True(File.Exists(sourcePluginPath), $"Plugin not found: {sourcePluginPath}");
        File.Copy(sourcePluginPath, targetPluginPath, overwrite: true);
        
        using Process proc = Process.Start(psi);
        Assert.NotNull(proc);
        
        String stdout = proc.StandardOutput.ReadToEnd();
        String stderr = proc.StandardError.ReadToEnd();
        proc.WaitForExit(30_000);

        Assert.True(proc.HasExited, "Process did not exit within timeout");
        Assert.True(proc.ExitCode == 0, $"Process exited with code {proc.ExitCode}.\nSTDOUT:\n{stdout}\nSTDERR:\n{stderr}");
        Assert.True(stdout.Contains($"Hello from app::PrintHello (cdecl) [{platform}]"), $"Process did not print hello message.\nSTDOUT:\n{stdout}\nSTDERR:\n{stderr}");
    }

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

    private static String GetOutputFolder()
    {
        // Tests project OutDir points to ..\..\Output\Tests\
        // The native apps are expected in subfolders x64/ and x86/ relative to that.
        return AppContext.BaseDirectory.TrimEnd(Path.DirectorySeparatorChar, Path.AltDirectorySeparatorChar);
    }
}
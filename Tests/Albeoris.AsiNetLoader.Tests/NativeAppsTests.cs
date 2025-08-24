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

    private static void RunAndAssertExitCodeZero(String platform, String exeRelativePath)
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

        using Process proc = Process.Start(psi)!;
        Assert.NotNull(proc);
        String stdout = proc.StandardOutput.ReadToEnd();
        String stderr = proc.StandardError.ReadToEnd();
        proc.WaitForExit(30_000);

        Assert.True(proc.HasExited, "Process did not exit within timeout");
        Int32 code = proc.ExitCode;
        Assert.True(code == 0, $"Process exited with code {code}.\nSTDOUT:\n{stdout}\nSTDERR:\n{stderr}");
        Assert.True(stdout.Contains($"Hello from app::PrintHello (cdecl) [{platform}]"), $"Process did not print hello message.\nSTDOUT:\n{stdout}\nSTDERR:\n{stderr}");
    }

    private static String GetOutputFolder()
    {
        // Tests project OutDir points to ..\..\Output\Tests\
        // The native apps are expected in subfolders x64/ and x86/ relative to that.
        return AppContext.BaseDirectory.TrimEnd(Path.DirectorySeparatorChar, Path.AltDirectorySeparatorChar);
    }
}
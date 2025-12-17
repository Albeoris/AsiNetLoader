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
        RunAndAssertExitCodeZero(TestProcessContext.CreateX64());
    }

    [Fact(DisplayName = "Run x86 native test app")]
    public void Run_X86_Native_App()
    {
        RunAndAssertExitCodeZero(TestProcessContext.CreateX86());
    }
    
    [Fact(DisplayName = "Hook x64 native test app")]
    public void Hook_X64_Native_App()
    {
        HookAndAssertExitCodeZero(TestProcessContext.CreateX64());
    }

    [Fact(DisplayName = "Hook x86 native test app")]
    public void Hook_X86_Native_App()
    {
        HookAndAssertExitCodeZero(TestProcessContext.CreateX86());
    }

    private static void RunAndAssertExitCodeZero(TestProcessContext ctx)
    {
        using Process proc = Process.Start(ctx.StartInfo);
        Assert.NotNull(proc);
        
        String stdout = proc.StandardOutput.ReadToEnd();
        String stderr = proc.StandardError.ReadToEnd();
        proc.WaitForExit(30_000);

        Assert.True(proc.HasExited, "Process did not exit within timeout");
        Assert.True(proc.ExitCode == 0, $"Process exited with code {proc.ExitCode}.\nSTDOUT:\n{stdout}\nSTDERR:\n{stderr}");
        Assert.True(stdout.Contains($"Hello from app::PrintHello (cdecl) [{ctx.Platform}]"), $"Process did not print hello message.\nSTDOUT:\n{stdout}\nSTDERR:\n{stderr}");

        ctx.StdOut = stdout;
        ctx.StdErr = stderr;
    }
    
    private static void HookAndAssertExitCodeZero(TestProcessContext ctx)
    {
        String workingDirectory = Path.GetDirectoryName(ctx.StartInfo.FileName);
        Assert.NotNull(workingDirectory);

        String pluginsFolder = Path.Combine(workingDirectory, "Plugins");
        String sourcePluginFolder = Path.GetFullPath(Path.Combine(workingDirectory, "..", "..", "Plugin", ctx.Platform));
        Assert.True(Directory.Exists(sourcePluginFolder), $"Plugin folder not found: {sourcePluginFolder}");
        
        CreateJunctionPoint(pluginsFolder, sourcePluginFolder);

        RunAndAssertExitCodeZero(ctx);

        Assert.True(ctx.StdOut.Contains("DllMain: DLL_PROCESS_ATTACH"), $"Process did not print DLL_PROCESS_ATTACH.\nSTDOUT:\n{ctx.StdOut}\nSTDERR:\n{ctx.StdErr}");
    }

    private static void CreateJunctionPoint(String sourceDirectory, String targetPath)
    {
        // Remove existing Plugins folder/link if exists
        if (Directory.Exists(sourceDirectory))
        {
            if (Directory.ResolveLinkTarget(sourceDirectory, returnFinalTarget: false) != null)
                Directory.Delete(sourceDirectory, recursive: false);
            else
                Directory.Delete(sourceDirectory, recursive: true);
        }
        
        // Create junction using mklink command (doesn't require admin rights)
        ProcessStartInfo mklinkPsi = new ProcessStartInfo
        {
            FileName = "cmd.exe",
            Arguments = $"/c mklink /J \"{sourceDirectory}\" \"{targetPath}\"",
            UseShellExecute = false,
            RedirectStandardOutput = true,
            RedirectStandardError = true,
            CreateNoWindow = true
        };
        
        using (Process mklinkProc = Process.Start(mklinkPsi))
        {
            Assert.NotNull(mklinkProc);
            mklinkProc.WaitForExit();
            Assert.True(mklinkProc.ExitCode == 0, $"Failed to create junction: {mklinkProc.StandardError.ReadToEnd()}");
        }
    }
}
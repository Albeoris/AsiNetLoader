using System;
using System.Diagnostics;
using System.IO;
using System.IO.Enumeration;
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
    
    [Fact(DisplayName = "Failover log if common place is not available")]
    public void Hook_X64_And_Check_LogFailoverAlg()
    {
        TestProcessContext ctx = TestProcessContext.CreateX64();
        String primaryLogPath = ctx.GetPrimaryLogPath();
        String secondaryLogPath = ctx.GetSecondaryLogPath(ctx.StartInfo.FileName);
        
        Cleanup();

        try
        {
            HookAndAssertExitCodeZero(ctx);
            Assert.True(File.Exists(primaryLogPath), $"Primary log file not found: {primaryLogPath}");

            File.Delete(primaryLogPath);
            Directory.CreateDirectory(primaryLogPath); // Take the log location to prevent file creation
            HookAndAssertExitCodeZero(ctx);
            Assert.True(File.Exists(secondaryLogPath), $"Secondary log file not found: {secondaryLogPath}");
            
            File.Delete(secondaryLogPath);
            Directory.CreateDirectory(secondaryLogPath); // Take the log location to prevent file creation
            HookAndAssertExitCodeZero(ctx);
            Assert.True(ctx.StdOut.Contains(".NET Runtime initialized successfully"), $"Failed to failover logs to the console.\nSTDOUT:\n{ctx.StdOut}\nSTDERR:\n{ctx.StdErr}");
        }
        finally
        {
            Cleanup();
        }

        return;

        void Cleanup()
        {
            if (Directory.Exists(primaryLogPath))
                Directory.Delete(primaryLogPath);
            else if (File.Exists(primaryLogPath))
                File.Delete(primaryLogPath);

            if (Directory.Exists(secondaryLogPath))
                Directory.Delete(secondaryLogPath);
            else if (File.Exists(secondaryLogPath))
                File.Delete(secondaryLogPath);
        }
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
        String workingDirectory = ctx.StartInfo.WorkingDirectory;

        String pluginsFolder = ctx.PluginsDirectoryPath;
        String sourcePluginFolder = Path.GetFullPath(Path.Combine(workingDirectory, "..", "..", "Plugin", ctx.Platform));
        Assert.True(Directory.Exists(sourcePluginFolder), $"Plugin folder not found: {sourcePluginFolder}");

        CreateJunctionPoint(pluginsFolder, sourcePluginFolder);

        RunAndAssertExitCodeZero(ctx);

        String logContent = ctx.ReadAllOutputText(ctx.StartInfo.FileName);
        Assert.True(logContent.Contains("InitializeASI"), $"Failed to hook process.\nSTDOUT:\n{ctx.StdOut}\nSTDERR:\n{ctx.StdErr}\nLOG:\n{logContent}");
        Assert.True(logContent.Contains(".NET Runtime initialized successfully"), $"Failed to initialize .NET Runtime.\nSTDOUT:\n{ctx.StdOut}\nSTDERR:\n{ctx.StdErr}\nLOG:\n{logContent}");
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
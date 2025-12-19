using System;
using System.Runtime.InteropServices;

namespace Albeoris.TestNetApp
{
    internal class Program
    {
        public static void Main(String[] args)
        {
            String timestamp = DateTime.Now.ToString("yyyy-MM-dd HH:mm:ss.fff");
            Console.WriteLine($"[{timestamp}] Starting .NET 8 test app...");

            Guid guid = Guid.Empty;
            Int32 hr = DirectInput8Create(IntPtr.Zero, 0x0800, ref guid, out _, IntPtr.Zero);
            Console.WriteLine($"DirectInput8Create returned: 0x{hr:X8}");

            PrintHello();

            timestamp = DateTime.Now.ToString("yyyy-MM-dd HH:mm:ss.fff");
            Console.WriteLine($"[{timestamp}] .NET 8 app finished.");
        }

        private static void PrintHello()
        {
            if (Environment.Is64BitProcess)
                Console.WriteLine("Hello from app::PrintHello (cdecl) [x64]");
            else
                Console.WriteLine("Hello from app::PrintHello (cdecl) [x86]");
        }
        
        [DllImport("dinput8.dll", CallingConvention = CallingConvention.StdCall)]
        private static extern Int32 DirectInput8Create(
            IntPtr hinst,
            UInt32 dwVersion,
            ref Guid riidltf,
            out IntPtr ppvOut,
            IntPtr punkOuter);
    }
}

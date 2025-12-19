# AsiNetLoader
Universal ASI loader that boots .NET 8+ via hostfxr inside native x86/x64 applications, enabling C# plugins with runtime hooks.

```
[ TestNativeApp.exe ]
           │
           ▼
    loads "dinput8.dll"
           │
           ▼
[ AsiNetLoader (proxy dinput8.dll) ]
           │
           │-- bootstrap hostfxr
           │-- load Plugins\*.dll
           │-- call Plugin.Init()
           │-- set ReadyEvent
           ▼
[ SamplePlugin.dll ]
           │
           │-- detour cdecl PrintHello
           │-- trampoline -> original
           ▼
[ Console ]
   ">>> Hooked from C# by SamplePlugin <<<"
```

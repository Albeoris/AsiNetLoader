export module GamePluginHost:Platform;

// Platform-wide character type for hostfxr APIs
#ifdef _WIN32
using char_t = wchar_t;
#else
using char_t = char;
#endif
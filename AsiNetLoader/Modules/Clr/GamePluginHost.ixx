// GamePluginHost.ixx - Primary module interface for the GamePluginHost module.
export module GamePluginHost;

// Standard library imports
import <filesystem>;
import <fstream>;
import <mutex>;
import <string>;
import <vector>;
import <format>;    // For std::format (C++20) used in message formatting
import <cassert>;   // For assertions (used internally)

// Import internal partitions of this module
export import :Platform;
export import :DotNetPluginLoader;

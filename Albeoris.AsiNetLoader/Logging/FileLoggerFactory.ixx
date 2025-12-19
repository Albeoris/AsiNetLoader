module;

#include <memory>
#include <string>
#include <filesystem>
#include <Windows.h>
#include <ShlObj.h>

export module Albeoris.AsiNetLoader.Logging:FileLoggerFactory;

import :ILogger;
import :ILoggerFactory;
import :FileLogger;
import :ConsoleLogger;

export namespace Albeoris::AsiNetLoader::Logging
{
    /// <summary>
    /// Factory for creating file-based loggers with fallback to console
    /// </summary>
    class FileLoggerFactory : public ILoggerFactory
    {
    private:
        std::string logFilePath;
        bool useConsole;

        /// <summary>
        /// Try to create a log file at the specified path
        /// </summary>
        bool TryCreateLogFile(const std::filesystem::path& directory, const std::string& fileName, std::string& outPath)
        {
            try
            {
                // Create directory if it doesn't exist
                if (!std::filesystem::exists(directory))
                {
                    std::filesystem::create_directories(directory);
                }

                // Build full path
                std::filesystem::path fullPath = directory / fileName;
                outPath = fullPath.string();

                // Try to create/open the file
                std::ofstream testFile(outPath, std::ios::app);
                if (testFile.is_open())
                {
                    testFile.close();
                    return true;
                }
            }
            catch (...)
            {
                // Failed to create file or directory
            }

            return false;
        }

        /// <summary>
        /// Get the path to LocalAppData\Albeoris\AsiNetLoader\<exe name>\Logs\
        /// </summary>
        std::filesystem::path GetLocalAppDataLogPath()
        {
            wchar_t* localAppDataPath = nullptr;
            if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, nullptr, &localAppDataPath)))
            {
                std::filesystem::path basePath(localAppDataPath);
                CoTaskMemFree(localAppDataPath);

                // Get current executable name
                wchar_t exePath[MAX_PATH];
                GetModuleFileNameW(nullptr, exePath, MAX_PATH);
                std::filesystem::path exeName = std::filesystem::path(exePath).stem();

                return basePath / "Albeoris" / "AsiNetLoader" / exeName / "Logs";
            }

            return {};
        }

    public:
        /// <summary>
        /// Create a logger factory
        /// </summary>
        /// <param name="dllPath">Path to the DLL file (used to determine log location)</param>
        explicit FileLoggerFactory(const std::filesystem::path& dllPath)
            : useConsole(true)
        {
            std::filesystem::path dllDirectory = dllPath.parent_path();
            std::string fileName = "Albeoris.AsiNetLoader.log";

            // Try 1: Create log file in DLL directory/Logs/
            std::filesystem::path primaryLogDir = dllDirectory / "Logs";
            if (TryCreateLogFile(primaryLogDir, fileName, logFilePath))
            {
                useConsole = false;
                return;
            }

            // Try 2: Create log file in LocalAppData
            std::filesystem::path localAppDataLogDir = GetLocalAppDataLogPath();
            if (!localAppDataLogDir.empty() && TryCreateLogFile(localAppDataLogDir, fileName, logFilePath))
            {
                useConsole = false;
                return;
            }

            // Try 3: Fallback to console logger
            useConsole = true;
        }

        /// <summary>
        /// Create a logger instance
        /// </summary>
        std::shared_ptr<ILogger> CreateLogger() override
        {
            if (useConsole)
            {
                return std::make_shared<ConsoleLogger>();
            }
            else
            {
                auto logger = std::make_shared<FileLogger>(logFilePath);
                if (!logger->IsOpen())
                {
                    // Fallback to console if file couldn't be opened
                    return std::make_shared<ConsoleLogger>();
                }
                return logger;
            }
        }

        /// <summary>
        /// Get the log file path (empty if using console)
        /// </summary>
        std::string GetLogFilePath() const { return useConsole ? "" : logFilePath; }

        /// <summary>
        /// Check if using console logger
        /// </summary>
        bool IsUsingConsole() const { return useConsole; }
    };
}

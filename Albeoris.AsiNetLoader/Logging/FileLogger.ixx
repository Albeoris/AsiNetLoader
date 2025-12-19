module;

#include <string>
#include <fstream>
#include <mutex>
#include <chrono>
#include <iomanip>
#include <sstream>

export module Albeoris.AsiNetLoader.Logging:FileLogger;

import :ILogger;

export namespace Albeoris::AsiNetLoader::Logging
{
    /// <summary>
    /// Logger implementation that writes to a file
    /// </summary>
    class FileLogger : public ILogger
    {
    private:
        std::ofstream fileStream;
        std::mutex mutex;
        bool isOpen;

        std::string GetTimestamp()
        {
            auto now = std::chrono::system_clock::now();
            auto time_t_now = std::chrono::system_clock::to_time_t(now);
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

            std::tm tm_now;
            localtime_s(&tm_now, &time_t_now);

            std::ostringstream oss;
            oss << std::put_time(&tm_now, "%Y-%m-%d %H:%M:%S");
            oss << '.' << std::setfill('0') << std::setw(3) << ms.count();
            return oss.str();
        }

    public:
        explicit FileLogger(const std::string& filePath)
            : isOpen(false)
        {
            fileStream.open(filePath, std::ios::out | std::ios::trunc);
            isOpen = fileStream.is_open();
        }

        ~FileLogger() override
        {
            if (isOpen)
            {
                std::lock_guard<std::mutex> lock(mutex);
                fileStream.close();
            }
        }

        bool IsOpen() const { return isOpen; }

        void Write(const std::string& message) override
        {
            if (!isOpen) return;

            std::lock_guard<std::mutex> lock(mutex);
            fileStream << message;
        }

        void WriteLine(const std::string& message) override
        {
            if (!isOpen) return;

            std::lock_guard<std::mutex> lock(mutex);
            fileStream << "[" << GetTimestamp() << "] " << message << std::endl;
        }

        void Flush() override
        {
            if (!isOpen) return;

            std::lock_guard<std::mutex> lock(mutex);
            fileStream.flush();
        }
    };
}

module;

#include <string>

export module AsiNetLoader.Logging:ILogger;

export namespace AsiNetLoader::Logging
{
    /// <summary>
    /// Interface for logging messages
    /// </summary>
    class ILogger
    {
    public:
        virtual ~ILogger() = default;

        /// <summary>
        /// Write a message to the log
        /// </summary>
        /// <param name="message">The message to log</param>
        virtual void Write(const std::string& message) = 0;

        /// <summary>
        /// Write a message with a newline to the log
        /// </summary>
        /// <param name="message">The message to log</param>
        virtual void WriteLine(const std::string& message) = 0;

        /// <summary>
        /// Flush any buffered log entries
        /// </summary>
        virtual void Flush() = 0;
    };
}

module;

#include <memory>

export module AsiNetLoader.Logging:ILoggerFactory;

import :ILogger;

export namespace AsiNetLoader::Logging
{
    /// <summary>
    /// Interface for creating logger instances
    /// </summary>
    class ILoggerFactory
    {
    public:
        virtual ~ILoggerFactory() = default;

        /// <summary>
        /// Create a logger instance
        /// </summary>
        /// <returns>A shared pointer to the logger instance</returns>
        virtual std::shared_ptr<ILogger> CreateLogger() = 0;
    };
}

module;

#include <string>
#include <iostream>

export module Albeoris.AsiNetLoader.Logging:ConsoleLogger;

import :ILogger;

export namespace Albeoris::AsiNetLoader::Logging
{
    /// <summary>
    /// Logger implementation that writes to console
    /// </summary>
    class ConsoleLogger : public ILogger
    {
    public:
        void Write(const std::string& message) override
        {
            std::cout << message;
        }

        void WriteLine(const std::string& message) override
        {
            std::cout << message << std::endl;
        }

        void Flush() override
        {
            std::cout.flush();
        }
    };
}

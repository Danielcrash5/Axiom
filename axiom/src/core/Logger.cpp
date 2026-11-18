/**
 * @file axiom/src/core/Logger.cpp
 * @brief Implements the logger module.
 */

#include "axiom/core/Logger.h"
#include <iostream>

namespace axiom {

    namespace {
        /**
         * @brief Converts this value to string.
         * @param level The level used by this operation.
         * @return The result of the operation.
         */
        const char *toString(LogLevel level) {
            switch (level) {
            case LogLevel::trace:
                return "TRACE";
            case LogLevel::debug:
                return "DEBUG";
            case LogLevel::info:
                return "INFO";
            case LogLevel::warn:
                return "WARN";
            case LogLevel::error:
                return "ERROR";
            case LogLevel::fatal:
                return "FATAL";
            default:
                return "UNKNOWN";
            }
        }

        /**
         * @brief Returns the color.
         * @param level The level used by this operation.
         * @return The result of the operation.
         */
        const char *getColor(LogLevel level) {
            switch (level) {
            case LogLevel::trace:
                return "\033[37m"; // white
            case LogLevel::debug:
                return "\033[36m"; // cyan
            case LogLevel::info:
                return "\033[32m"; // green
            case LogLevel::warn:
                return "\033[33m"; // yellow
            case LogLevel::error:
                return "\033[31m"; // red
            case LogLevel::fatal:
                return "\033[41m"; // red background
            default:
                return "\033[0m";
            }
        }
    } // namespace

#ifdef AXIOM_ENABLE_CONSOLE_LOG
    void ConsoleSink::write(LogLevel level, const std::string &message) {
        std::cout << getColor(level) << "[" << toString(level) << "] "
                  << message << "\033[0m" << std::endl;

        if (level == LogLevel::fatal)
            std::terminate();
    }

#endif

    Logger &Logger::get() {
        static Logger instance;
        return instance;
    }

    void Logger::setLevel(LogLevel level) { m_level = level; }

    void Logger::addSink(std::shared_ptr<LogSink> sink) {
        std::lock_guard lock(m_mutex);
        m_sinks.emplace_back(sink);
    }

} // namespace axiom
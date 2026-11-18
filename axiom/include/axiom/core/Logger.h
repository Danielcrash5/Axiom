/**
 * @file axiom/include/axiom/core/Logger.h
 * @brief Declares the public interface for the logger.
 */

#pragma once

#include "LogSink.h"

#include <format>
#include <memory>
#include <mutex>
#include <vector>

namespace axiom {

#ifdef AXIOM_ENABLE_CONSOLE_LOG
    /**
     * @brief Provides the console sink functionality.
     * @tparam ConsoleSink Template type used by this type.
     */
    class ConsoleSink : public LogSink {
      public:
        /**
         * @brief Writes data to the destination.
         * @param level The level used by this operation.
         * @param message The message used by this operation.
         */
        void write(LogLevel level, const std::string &message) override;
    };
#endif

    /**
     * @brief Sends log messages to the configured output sinks.
     * @tparam Logger Template type used by this type.
     */
    class Logger {
      public:
        /**
         * @brief Returns the requested value.
         * @return The result of the operation.
         */
        static Logger &get();

        /**
         * @brief Sets level.
         * @param level The level used by this operation.
         */
        void setLevel(LogLevel level);
        /**
         * @brief Adds sink.
         * @param sink The sink used by this operation.
         */
        void addSink(std::shared_ptr<LogSink> sink);

        /**
         * @brief Performs the log operation.
         * @param level The level used by this operation.
         * @param fmt The fmt used by this operation.
         * @param args The args used by this operation.
         */
        template <typename... Args>
        void log(LogLevel level, std::format_string<Args...> fmt,
                 Args &&...args) {
            if (level < m_level)
                return;

            std::string message = std::format(fmt, std::forward<Args>(args)...);

            std::lock_guard lock(m_mutex);

            for (auto &sink : m_sinks)
                sink->write(level, message);
        }

      private:
        /**
         * @brief Creates an instance of logger.
         */
        Logger() = default;
        /**
         * @brief Releases resources owned by Logger.
         */
        ~Logger() = default;

      private:
        /** @brief Stores the level. */
        LogLevel m_level = LogLevel::trace;
        /** @brief Stores the sinks. */
        std::vector<std::shared_ptr<LogSink>> m_sinks;
        /** @brief Stores the mutex. */
        std::mutex m_mutex;
    };

} // namespace axiom

/** @brief Provides the AXIOM_TRACE macro. */
#define AXIOM_TRACE(...)                                                       \
    axiom::Logger::get().log(axiom::LogLevel::trace, __VA_ARGS__)
/** @brief Provides the AXIOM_DEBUG macro. */
#define AXIOM_DEBUG(...)                                                       \
    axiom::Logger::get().log(axiom::LogLevel::debug, __VA_ARGS__)
/** @brief Provides the AXIOM_INFO macro. */
#define AXIOM_INFO(...)                                                        \
    axiom::Logger::get().log(axiom::LogLevel::info, __VA_ARGS__)
/** @brief Provides the AXIOM_WARN macro. */
#define AXIOM_WARN(...)                                                        \
    axiom::Logger::get().log(axiom::LogLevel::warn, __VA_ARGS__)
/** @brief Provides the AXIOM_ERROR macro. */
#define AXIOM_ERROR(...)                                                       \
    axiom::Logger::get().log(axiom::LogLevel::error, __VA_ARGS__)
/** @brief Provides the AXIOM_FATAL macro. */
#define AXIOM_FATAL(...)                                                       \
    axiom::Logger::get().log(axiom::LogLevel::fatal, __VA_ARGS__)

/** @brief Provides the AXIOM_ASSERT macro. */
#define AXIOM_ASSERT(x, ...)                                                   \
    do {                                                                       \
        if (!(x)) {                                                            \
            AXIOM_ERROR("Assertion Failed: " __VA_ARGS__);                     \
            std::abort();                                                      \
        }                                                                      \
    } while (0)

/** @brief Provides the AXIOM_ASSERT_FATAL macro. */
#define AXIOM_ASSERT_FATAL(x, ...)                                             \
    do {                                                                       \
        if (!(x)) {                                                            \
            AXIOM_FATAL("Assertion Failed: " __VA_ARGS__);                     \
            std::abort();                                                      \
        }                                                                      \
    } while (0)
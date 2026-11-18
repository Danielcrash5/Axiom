/**
 * @file axiom/include/axiom/core/LogSink.h
 * @brief Declares the public interface for the log sink.
 */

#pragma once

#include <string>

namespace axiom {

    /**
     * @brief Defines the available log level values.
     * @tparam LogLevel Template type used by this type.
     */
    enum class LogLevel { trace, debug, info, warn, error, fatal };

    /**
     * @brief Provides the log sink functionality.
     * @tparam LogSink Template type used by this type.
     */
    class LogSink {
      public:
        /**
         * @brief Releases resources owned by LogSink.
         */
        virtual ~LogSink() = default;
        /**
         * @brief Writes data to the destination.
         * @param level The level used by this operation.
         * @param message The message used by this operation.
         */
        virtual void write(LogLevel level, const std::string &message) = 0;
    };

} // namespace axiom
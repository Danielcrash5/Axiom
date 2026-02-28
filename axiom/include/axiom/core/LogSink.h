#pragma once

#include <string>

namespace axiom {

    enum class LogLevel {
        Trace,
        Debug,
        Info,
        Warn,
        Error,
        Fatal
    };

    class LogSink {
    public:
        virtual ~LogSink() = default;
        virtual void Write(LogLevel level, const std::string& message) = 0;
    };

}
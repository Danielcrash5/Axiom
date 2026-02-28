#include "axiom/core/Logger.h"
#include <iostream>

namespace axiom {

	namespace {
		const char* ToString(LogLevel level) {
			switch (level) {
			case LogLevel::Trace: return "TRACE";
			case LogLevel::Debug: return "DEBUG";
			case LogLevel::Info:  return "INFO";
			case LogLevel::Warn:  return "WARN";
			case LogLevel::Error: return "ERROR";
			case LogLevel::Fatal: return "FATAL";
			default: return "UNKNOWN";
			}
		}

		const char* GetColor(LogLevel level) {
			switch (level) {
			case LogLevel::Trace: return "\033[37m"; // white
			case LogLevel::Debug: return "\033[36m"; // cyan
			case LogLevel::Info:  return "\033[32m"; // green
			case LogLevel::Warn:  return "\033[33m"; // yellow
			case LogLevel::Error: return "\033[31m"; // red
			case LogLevel::Fatal: return "\033[41m"; // red background
			default: return "\033[0m";
			}
		}
	}

#ifdef AXIOM_ENABLE_CONSOLE_LOG
	void ConsoleSink::Write(LogLevel level, const std::string& message) {
		std::cout
			<< GetColor(level)
			<< "[" << ToString(level) << "] "
			<< message
			<< "\033[0m"
			<< std::endl;

		if (level == LogLevel::Fatal)
			std::terminate();
	}

#endif

	Logger& Logger::Get() {
		static Logger instance;
		return instance;
	}

	void Logger::SetLevel(LogLevel level) {
		m_Level = level;
	}

	void Logger::AddSink(std::unique_ptr<LogSink> sink) {
		std::lock_guard lock(m_Mutex);
		m_Sinks.emplace_back(std::move(sink));
	}

}
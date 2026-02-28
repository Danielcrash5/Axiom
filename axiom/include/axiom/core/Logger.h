#pragma once

#include "LogSink.h"

#include <vector>
#include <memory>
#include <mutex>
#include <format>

namespace axiom {

#ifdef AXIOM_ENABLE_CONSOLE_LOG
	class ConsoleSink : public LogSink {
	public:
		void Write(LogLevel level, const std::string& message) override;
	};
#endif

	class Logger {
	public:
		static Logger& Get();

		void SetLevel(LogLevel level);
		void AddSink(std::unique_ptr<LogSink> sink);

		template<typename... Args>
		void Log(LogLevel level,
				 std::format_string<Args...> fmt,
				 Args&&... args) {
			if (level < m_Level)
				return;

			std::string message =
				std::format(fmt, std::forward<Args>(args)...);

			std::lock_guard lock(m_Mutex);

			for (auto& sink : m_Sinks)
				sink->Write(level, message);
		}

	private:
		Logger() = default;
		~Logger() = default;

	private:
		LogLevel m_Level = LogLevel::Trace;
		std::vector<std::unique_ptr<LogSink>> m_Sinks;
		std::mutex m_Mutex;
	};

}

#define ENG_TRACE(...) axiom::Logger::Get().Log(axiom::LogLevel::Trace, __VA_ARGS__)
#define ENG_DEBUG(...) axiom::Logger::Get().Log(axiom::LogLevel::Debug, __VA_ARGS__)
#define ENG_INFO(...)  axiom::Logger::Get().Log(axiom::LogLevel::Info,  __VA_ARGS__)
#define ENG_WARN(...)  axiom::Logger::Get().Log(axiom::LogLevel::Warn,  __VA_ARGS__)
#define ENG_ERROR(...) axiom::Logger::Get().Log(axiom::LogLevel::Error, __VA_ARGS__)
#define ENG_FATAL(...) axiom::Logger::Get().Log(axiom::LogLevel::Fatal, __VA_ARGS__)
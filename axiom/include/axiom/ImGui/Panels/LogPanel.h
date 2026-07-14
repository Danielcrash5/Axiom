#pragma once

#include "axiom/ImGui/IImGuiPanel.h"
#include "axiom/core/Logger.h"
#include "axiom/core/Logsink.h"

#include <imgui.h>

#include <chrono>
#include <deque>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>
#include <vector>
#include <iomanip>

namespace axiom {
    class ImGuiPanelLogsink : public LogSink {
      public:
        struct LogEntry {
            LogLevel Level;
            std::string Timestamp;
            std::string Message;
        };

      public:
        void Write(LogLevel level, const std::string &message) override {
            std::lock_guard<std::mutex> lock(m_Mutex);

            m_PendingLogs.push_back({level, CreateTimestamp(), message});
        }

        std::vector<LogEntry> ConsumeLogs() {
            std::lock_guard<std::mutex> lock(m_Mutex);

            std::vector<LogEntry> result;
            result.swap(m_PendingLogs);

            return result;
        }

      private:
        static std::string CreateTimestamp() {
            auto now = std::chrono::system_clock::now();
            auto tt = std::chrono::system_clock::to_time_t(now);

            std::tm tm{};

#ifdef _WIN32
            localtime_s(&tm, &tt);
#else
            localtime_r(&tt, &tm);
#endif

            std::stringstream ss;

            ss << std::setfill('0') << std::setw(2) << tm.tm_hour << ":"
               << std::setw(2) << tm.tm_min << ":" << std::setw(2) << tm.tm_sec;

            return ss.str();
        }

      private:
        std::mutex m_Mutex;
        std::vector<LogEntry> m_PendingLogs;
    };

    class LogPanel : public IImGuiPanel {
      public:
        explicit LogPanel(std::shared_ptr<ImGuiPanelLogsink> logSink)
            : IImGuiPanel("Log"), m_LogSink(std::move(logSink)) {}

      public:
        void OnUpdate(double deltaTime) override {
            auto newLogs = m_LogSink->ConsumeLogs();

            if (!newLogs.empty()) {
                for (auto &log : newLogs)
                    m_Logs.emplace_back(std::move(log));

                m_ScrollToBottom = true;
            }

            while (m_Logs.size() > MaxLogs)
                m_Logs.pop_front();
        }

        void OnImGuiRender() override {
            DrawToolbar();

            ImGui::Separator();

            ImGui::BeginChild("LogRegion", ImVec2(0, 0), true,
                              ImGuiWindowFlags_HorizontalScrollbar);

            if (m_CopyRequested)
                m_CopyBuffer.clear();

            for (const auto &log : m_Logs) {
                if (!ShouldShow(log.Level))
                    continue;

                const char *levelStr = GetLevelString(log.Level);

                std::string searchable =
                    std::string(levelStr) + " " + log.Message;

                if (!m_Filter.PassFilter(searchable.c_str()))
                    continue;

                DrawLogEntry(log);

                if (m_CopyRequested) {
                    m_CopyBuffer += "[";
                    m_CopyBuffer += log.Timestamp;
                    m_CopyBuffer += "] ";

                    m_CopyBuffer += "[";
                    m_CopyBuffer += levelStr;
                    m_CopyBuffer += "] ";

                    m_CopyBuffer += log.Message;
                    m_CopyBuffer += "\n";
                }
            }

            if (m_CopyRequested) {
                ImGui::SetClipboardText(m_CopyBuffer.c_str());
                m_CopyRequested = false;
            }

            if (m_AutoScroll && m_ScrollToBottom) {
                ImGui::SetScrollHereY(1.0f);
                m_ScrollToBottom = false;
            }

            ImGui::EndChild();
        }

      private:
        void DrawToolbar() {
            if (ImGui::Button("Clear"))
                m_Logs.clear();

            ImGui::SameLine();

            if (ImGui::Button("Copy"))
                m_CopyRequested = true;

            ImGui::SameLine();

            ImGui::Checkbox("Auto Scroll", &m_AutoScroll);

            ImGui::Separator();

            m_Filter.Draw("Search", 250.0f);

            ImGui::Separator();

            ImGui::Checkbox("Trace", &m_ShowTrace);
            ImGui::SameLine();

            ImGui::Checkbox("Info", &m_ShowInfo);
            ImGui::SameLine();

            ImGui::Checkbox("Warn", &m_ShowWarn);
            ImGui::SameLine();

            ImGui::Checkbox("Error", &m_ShowError);
            ImGui::SameLine();

            ImGui::Checkbox("Fatal", &m_ShowFatal);
        }

        void DrawLogEntry(const ImGuiPanelLogsink::LogEntry &log) {
            ImGui::PushID(&log);

            ImGui::TextDisabled("[%s]", log.Timestamp.c_str());

            ImGui::SameLine();

            ImGui::TextColored(GetLevelColor(log.Level), "[%s]",
                               GetLevelString(log.Level));

            ImGui::SameLine();

            ImGui::TextWrapped("%s", log.Message.c_str());

            ImGui::PopID();
        }

        static const char *GetLevelString(LogLevel level) {
            switch (level) {
            case LogLevel::Trace:
                return "TRACE";
            case LogLevel::Info:
                return "INFO";
            case LogLevel::Warn:
                return "WARN";
            case LogLevel::Error:
                return "ERROR";
            case LogLevel::Fatal:
                return "FATAL";
            default:
                return "UNKNOWN";
            }
        }

        static ImVec4 GetLevelColor(LogLevel level) {
            switch (level) {
            case LogLevel::Trace:
                return ImVec4(0.60f, 0.60f, 0.60f, 1.0f);

            case LogLevel::Info:
                return ImVec4(0.35f, 0.75f, 1.00f, 1.0f);

            case LogLevel::Warn:
                return ImVec4(1.00f, 0.85f, 0.20f, 1.0f);

            case LogLevel::Error:
                return ImVec4(1.00f, 0.45f, 0.45f, 1.0f);

            case LogLevel::Fatal:
                return ImVec4(1.00f, 0.10f, 0.10f, 1.0f);

            default:
                return ImVec4(1, 1, 1, 1);
            }
        }

        bool ShouldShow(LogLevel level) const {
            switch (level) {
            case LogLevel::Trace:
                return m_ShowTrace;
            case LogLevel::Info:
                return m_ShowInfo;
            case LogLevel::Warn:
                return m_ShowWarn;
            case LogLevel::Error:
                return m_ShowError;
            case LogLevel::Fatal:
                return m_ShowFatal;
            default:
                return true;
            }
        }

      private:
        static constexpr size_t MaxLogs = 10000;

      private:
        std::shared_ptr<ImGuiPanelLogsink> m_LogSink;

        std::deque<ImGuiPanelLogsink::LogEntry> m_Logs;

        ImGuiTextFilter m_Filter;

        bool m_ShowTrace = true;
        bool m_ShowInfo = true;
        bool m_ShowWarn = true;
        bool m_ShowError = true;
        bool m_ShowFatal = true;

        bool m_AutoScroll = true;
        bool m_ScrollToBottom = false;

        bool m_CopyRequested = false;
        std::string m_CopyBuffer;
    };
} // namespace axiom
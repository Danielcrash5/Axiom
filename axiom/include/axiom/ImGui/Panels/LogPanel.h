/**
 * @file axiom/include/axiom/ImGui/Panels/LogPanel.h
 * @brief Declares the public interface for the log panel.
 */

#pragma once

#include "axiom/ImGui/IImGuiPanel.h"
#include "axiom/core/Logger.h"
#include "axiom/core/LogSink.h"

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
    /**
     * @brief Provides the ImGui panel logsink functionality.
     * @tparam ImGuiPanelLogsink Template type used by this type.
     */
    class ImGuiPanelLogsink : public LogSink {
      public:
        /** @brief Stores the data for the log entry. */
        struct LogEntry {
            /** @brief Stores the level. */
            LogLevel Level;
            /** @brief Stores the timestamp. */
            std::string Timestamp;
            /** @brief Stores the message. */
            std::string Message;
        };

      public:
        /**
         * @brief Writes data to the destination.
         * @param level The level used by this operation.
         * @param message The message used by this operation.
         */
        void write(LogLevel level, const std::string &message) override {
            std::lock_guard<std::mutex> lock(m_mutex);

            m_pendingLogs.push_back({level, createTimestamp(), message});
        }

        /**
         * @brief Performs the consume logs operation.
         * @return The result of the operation.
         */
        std::vector<LogEntry> consumeLogs() {
            std::lock_guard<std::mutex> lock(m_mutex);

            std::vector<LogEntry> result;
            result.swap(m_pendingLogs);

            return result;
        }

      private:
        /**
         * @brief Creates timestamp.
         * @return The result of the operation.
         */
        static std::string createTimestamp() {
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
        /** @brief Stores the mutex. */
        std::mutex m_mutex;
        /** @brief Stores the pending logs. */
        std::vector<LogEntry> m_pendingLogs;
    };

    /**
     * @brief Provides the log panel functionality.
     * @tparam LogPanel Template type used by this type.
     */
    class LogPanel : public IImGuiPanel {
      public:
        /**
         * @brief Creates an instance of log panel.
         * @param logSink The log sink used by this operation.
         */
        explicit LogPanel(std::shared_ptr<ImGuiPanelLogsink> logSink)
            : IImGuiPanel("Log"), m_logSink(std::move(logSink)) {}

      public:
        /**
         * @brief Handles the update callback.
         * @param deltaTime The delta time used by this operation.
         */
        void onUpdate(double deltaTime) override {
            auto newLogs = m_logSink->consumeLogs();

            if (!newLogs.empty()) {
                for (auto &log : newLogs)
                    m_logs.emplace_back(std::move(log));

                m_scrollToBottom = true;
            }

            while (m_logs.size() > maxLogs)
                m_logs.pop_front();
        }

        /**
         * @brief Handles the ImGui render callback.
         */
        void onImGuiRender() override {
            drawToolbar();

            ImGui::Separator();

            ImGui::BeginChild("LogRegion", ImVec2(0, 0), true,
                              ImGuiWindowFlags_HorizontalScrollbar);

            if (m_copyRequested)
                m_copyBuffer.clear();

            for (const auto &log : m_logs) {
                if (!shouldShow(log.Level))
                    continue;

                const char *levelStr = getLevelString(log.Level);

                std::string searchable =
                    std::string(levelStr) + " " + log.Message;

                if (!m_filter.PassFilter(searchable.c_str()))
                    continue;

                drawLogEntry(log);

                if (m_copyRequested) {
                    m_copyBuffer += "[";
                    m_copyBuffer += log.Timestamp;
                    m_copyBuffer += "] ";

                    m_copyBuffer += "[";
                    m_copyBuffer += levelStr;
                    m_copyBuffer += "] ";

                    m_copyBuffer += log.Message;
                    m_copyBuffer += "\n";
                }
            }

            if (m_copyRequested) {
                ImGui::SetClipboardText(m_copyBuffer.c_str());
                m_copyRequested = false;
            }

            if (m_autoScroll && m_scrollToBottom) {
                ImGui::SetScrollHereY(1.0f);
                m_scrollToBottom = false;
            }

            ImGui::EndChild();
        }

      private:
        /**
         * @brief Draws toolbar.
         */
        void drawToolbar() {
            if (ImGui::Button("Clear"))
                m_logs.clear();

            ImGui::SameLine();

            if (ImGui::Button("Copy"))
                m_copyRequested = true;

            ImGui::SameLine();

            ImGui::Checkbox("Auto Scroll", &m_autoScroll);

            ImGui::Separator();

            m_filter.Draw("Search", 250.0f);

            ImGui::Separator();

            ImGui::Checkbox("Trace", &m_showTrace);
            ImGui::SameLine();

            ImGui::Checkbox("Info", &m_showInfo);
            ImGui::SameLine();

            ImGui::Checkbox("Warn", &m_showWarn);
            ImGui::SameLine();

            ImGui::Checkbox("Error", &m_showError);
            ImGui::SameLine();

            ImGui::Checkbox("Fatal", &m_showFatal);
        }

        /**
         * @brief Draws log entry.
         * @param log The log used by this operation.
         */
        void drawLogEntry(const ImGuiPanelLogsink::LogEntry &log) {
            ImGui::PushID(&log);

            ImGui::TextDisabled("[%s]", log.Timestamp.c_str());

            ImGui::SameLine();

            ImGui::TextColored(getLevelColor(log.Level), "[%s]",
                               getLevelString(log.Level));

            ImGui::SameLine();

            ImGui::TextWrapped("%s", log.Message.c_str());

            ImGui::PopID();
        }

        /**
         * @brief Returns the level string.
         * @param level The level used by this operation.
         * @return The result of the operation.
         */
        static const char *getLevelString(LogLevel level) {
            switch (level) {
            case LogLevel::trace:
                return "TRACE";
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
         * @brief Returns the level color.
         * @param level The level used by this operation.
         * @return The result of the operation.
         */
        static ImVec4 getLevelColor(LogLevel level) {
            switch (level) {
            case LogLevel::trace:
                return ImVec4(0.60f, 0.60f, 0.60f, 1.0f);

            case LogLevel::info:
                return ImVec4(0.35f, 0.75f, 1.00f, 1.0f);

            case LogLevel::warn:
                return ImVec4(1.00f, 0.85f, 0.20f, 1.0f);

            case LogLevel::error:
                return ImVec4(1.00f, 0.45f, 0.45f, 1.0f);

            case LogLevel::fatal:
                return ImVec4(1.00f, 0.10f, 0.10f, 1.0f);

            default:
                return ImVec4(1, 1, 1, 1);
            }
        }

        /**
         * @brief Reports whether show.
         * @param level The level used by this operation.
         * @return The result of the operation.
         */
        bool shouldShow(LogLevel level) const {
            switch (level) {
            case LogLevel::trace:
                return m_showTrace;
            case LogLevel::info:
                return m_showInfo;
            case LogLevel::warn:
                return m_showWarn;
            case LogLevel::error:
                return m_showError;
            case LogLevel::fatal:
                return m_showFatal;
            default:
                return true;
            }
        }

      private:
        /** @brief Stores the max logs. */
        static constexpr size_t maxLogs = 10000;

      private:
        /** @brief Stores the log sink. */
        std::shared_ptr<ImGuiPanelLogsink> m_logSink;

        /** @brief Stores the logs. */
        std::deque<ImGuiPanelLogsink::LogEntry> m_logs;

        /** @brief Stores the filter. */
        ImGuiTextFilter m_filter;

        /** @brief Stores the show trace. */
        bool m_showTrace = true;
        /** @brief Stores the show info. */
        bool m_showInfo = true;
        /** @brief Stores the show warn. */
        bool m_showWarn = true;
        /** @brief Stores the show error. */
        bool m_showError = true;
        /** @brief Stores the show fatal. */
        bool m_showFatal = true;

        /** @brief Stores the auto scroll. */
        bool m_autoScroll = true;
        /** @brief Stores the scroll to bottom. */
        bool m_scrollToBottom = false;

        /** @brief Stores the copy requested. */
        bool m_copyRequested = false;
        /** @brief Stores the copy buffer. */
        std::string m_copyBuffer;
    };
} // namespace axiom
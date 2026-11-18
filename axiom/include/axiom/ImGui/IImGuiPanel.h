/**
 * @file axiom/include/axiom/ImGui/IImGuiPanel.h
 * @brief Declares the public interface for the i ImGui panel.
 */

#pragma once

#include <imgui.h>
#include <string>

namespace axiom {
    /**
     * @brief Defines the interface for the ImGui panel.
     * @tparam IImGuiPanel Template type used by this type.
     */
    class IImGuiPanel {
      public:
        /**
         * @brief Creates an instance of i ImGui panel.
         * @param name The name used by this operation.
         */
        IImGuiPanel(const std::string &name) : m_name(name) {}

        /**
         * @brief Releases resources owned by IImGuiPanel.
         */
        virtual ~IImGuiPanel() = default;

        /**
         * @brief Returns the name.
         * @return The result of the operation.
         */
        const std::string &getName() const { return m_name; }

        /**
         * @brief Performs the ImGui render operation.
         */
        inline void imGuiRender() {
            ImGui::PushID(m_name.c_str());
            ImGui::Begin(m_name.c_str());
            onImGuiRender();
            ImGui::End();
            ImGui::PopID();
        }

        /**
         * @brief Handles the update callback.
         * @param deltaTime The delta time used by this operation.
         */
        virtual void onUpdate(double deltaTime) {}

        /**
         * @brief Handles the ImGui render callback.
         */
        virtual void onImGuiRender() = 0;

      private:
        /** @brief Stores the name. */
        std::string m_name;
    };
} // namespace axiom
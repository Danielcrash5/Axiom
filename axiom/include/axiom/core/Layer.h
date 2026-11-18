/**
 * @file axiom/include/axiom/core/Layer.h
 * @brief Declares the public interface for the layer.
 */

#pragma once

#include <string>

#include "axiom/events/EventBus.h"

namespace axiom {

    class Event;

    /**
     * @brief Provides the layer functionality.
     * @tparam Layer Template type used by this type.
     */
    class Layer {
      public:
        /**
         * @brief Creates an instance of layer.
         * @param name The name used by this operation.
         */
        Layer(std::string name = "Layer") : m_Name(std::move(name)) {}

        /**
         * @brief Releases resources owned by Layer.
         */
        virtual ~Layer() = default;

        /**
         * @brief Handles the attach callback.
         */
        virtual void onAttach() {}
        /**
         * @brief Handles the detach callback.
         */
        virtual void onDetach() {}

        /**
         * @brief Handles the pre update callback.
         * @param dt The dt used by this operation.
         */
        virtual void onPreUpdate(double dt) {}
        /**
         * @brief Handles the post update callback.
         * @param dt The dt used by this operation.
         */
        virtual void onPostUpdate(double dt) {}
        /**
         * @brief Handles the fixed update callback.
         * @param dt The dt used by this operation.
         */
        virtual void onFixedUpdate(double dt) {}
        /**
         * @brief Handles the update callback.
         * @param dt The dt used by this operation.
         */
        virtual void onUpdate(double dt) {}

        /**
         * @brief Handles the render callback.
         * @param alpha The alpha used by this operation.
         */
        virtual void onRender(double alpha) {}
        /**
         * @brief Handles the ImGui render callback.
         */
        virtual void onImGuiRender() {}

        /**
         * @brief Sets event bus.
         * @param eventBus The event bus used by this operation.
         */
        void setEventBus(EventBus &eventBus) { m_EventBus = &eventBus; }

        /**
         * @brief Returns the name.
         * @return The result of the operation.
         */
        const std::string &getName() const { return m_Name; }

      protected:
        /** @brief Stores the name. */
        std::string m_Name;
        /** @brief Stores the event bus. */
        EventBus *m_EventBus = nullptr;
    };

} // namespace axiom
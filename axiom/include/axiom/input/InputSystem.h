/**
 * @file axiom/include/axiom/Input/InputSystem.h
 * @brief Declares the public interface for the input system.
 */

#pragma once

#include "InputContext.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace axiom {

    /**
     * @brief Evaluates input contexts and produces action state.
     * @tparam InputSystem Template type used by this type.
     */
    class InputSystem {
      public:
        /**
         * @brief Initializes the subsystem.
         */
        static void init();
        /**
         * @brief Updates the object state.
         */
        static void update();

        // Context Management
        /**
         * @brief Registers context.
         * @param context The context used by this operation.
         */
        static void
        registerContext(const std::shared_ptr<InputContext> &context);
        /**
         * @brief Adds context.
         * @param name The name used by this operation.
         */
        static void pushContext(const std::string &name);
        /**
         * @brief Removes context.
         */
        static void popContext();
        /**
         * @brief Clears contexts.
         */
        static void clearContexts();

        /**
         * @brief Returns the active context.
         * @return The result of the operation.
         */
        static InputContext *getActiveContext();

        // Convenience Wrappers
        /**
         * @brief Reports whether action pressed.
         * @param action The action used by this operation.
         * @return The result of the operation.
         */
        static bool isActionPressed(const std::string &action);
        /**
         * @brief Reports whether action just pressed.
         * @param action The action used by this operation.
         * @return The result of the operation.
         */
        static bool isActionJustPressed(const std::string &action);

        /**
         * @brief Returns the axis.
         * @param axis The axis used by this operation.
         * @return The result of the operation.
         */
        static float getAxis(const std::string &axis);
        /**
         * @brief Returns the axis2 d.
         * @param axis The axis used by this operation.
         * @return The result of the operation.
         */
        static glm::vec2 getAxis2D(const std::string &axis);

      private:
        static std::unordered_map<std::string, std::shared_ptr<InputContext>>
            /** @brief Stores the registered contexts. */
            sRegisteredContexts;
        /** @brief Stores the context stack. */
        static std::vector<std::shared_ptr<InputContext>> sContextStack;
    };

} // namespace axiom
/**
 * @file axiom/include/axiom/Input/InputContext.h
 * @brief Declares the public interface for the input context.
 */

#pragma once

#include "ActionMap.h"
#include <string>

namespace axiom {

    /**
     * @brief Provides the input context functionality.
     * @tparam InputContext Template type used by this type.
     */
    class InputContext {
      public:
        /**
         * @brief Creates an instance of input context.
         * @param name The name used by this operation.
         */
        explicit InputContext(const std::string &name) : m_name(name) {}

        /**
         * @brief Returns the name.
         * @return The result of the operation.
         */
        const std::string &getName() const { return m_name; }

        /**
         * @brief Returns the action map.
         * @return The result of the operation.
         */
        ActionMap &getActionMap() { return m_actionMap; }
        /**
         * @brief Returns the action map.
         * @return The result of the operation.
         */
        const ActionMap &getActionMap() const { return m_actionMap; }

      private:
        /** @brief Stores the name. */
        std::string m_name;
        /** @brief Stores the action map. */
        ActionMap m_actionMap;
    };

} // namespace axiom
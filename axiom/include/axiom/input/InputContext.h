#pragma once

#include "ActionMap.h"
#include <string>

namespace axiom {

    class InputContext {
      public:
        explicit InputContext(const std::string &name) : m_Name(name) {}

        const std::string &GetName() const { return m_Name; }

        ActionMap &GetActionMap() { return m_ActionMap; }
        const ActionMap &GetActionMap() const { return m_ActionMap; }

      private:
        std::string m_Name;
        ActionMap m_ActionMap;
    };

} // namespace axiom
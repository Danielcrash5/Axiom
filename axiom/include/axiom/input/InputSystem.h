#pragma once

#include <vector>
#include <memory>
#include <unordered_map>
#include <string>
#include "InputContext.h"

namespace axiom {

    class InputSystem {
    public:

        static void Init();
        static void Update();

        // Context Management
        static void RegisterContext(const std::shared_ptr<InputContext>& context);
        static void PushContext(const std::string& name);
        static void PopContext();
        static void ClearContexts();

        static InputContext* GetActiveContext();

        // Convenience Wrappers
        static bool IsActionPressed(const std::string& action);
        static bool IsActionJustPressed(const std::string& action);

        static float GetAxis(const std::string& axis);
        static glm::vec2 GetAxis2D(const std::string& axis);

    private:

        static std::unordered_map<std::string, std::shared_ptr<InputContext>> s_RegisteredContexts;
        static std::vector<std::shared_ptr<InputContext>> s_ContextStack;
    };

}
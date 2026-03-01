#include "axiom/input/InputSystem.h"
#include "axiom/input/Input.h"

namespace axiom {

    std::unordered_map<std::string, std::shared_ptr<InputContext>> InputSystem::s_RegisteredContexts;
    std::vector<std::shared_ptr<InputContext>> InputSystem::s_ContextStack;

    void InputSystem::Init() {
    }

    void InputSystem::Update() {
        Input::Update();
    }

    void InputSystem::RegisterContext(const std::shared_ptr<InputContext>& context) {
        s_RegisteredContexts[context->GetName()] = context;
    }

    void InputSystem::PushContext(const std::string& name) {
        auto it = s_RegisteredContexts.find(name);
        if (it != s_RegisteredContexts.end())
            s_ContextStack.push_back(it->second);
    }

    void InputSystem::PopContext() {
        if (!s_ContextStack.empty())
            s_ContextStack.pop_back();
    }

    void InputSystem::ClearContexts() {
        s_ContextStack.clear();
    }

    InputContext* InputSystem::GetActiveContext() {
        if (s_ContextStack.empty())
            return nullptr;

        return s_ContextStack.back().get();
    }

    // ==================== Wrappers ====================

    bool InputSystem::IsActionPressed(const std::string& action) {
        if (auto* ctx = GetActiveContext())
            return ctx->GetActionMap().IsActionPressed(action);

        return false;
    }

    bool InputSystem::IsActionJustPressed(const std::string& action) {
        if (auto* ctx = GetActiveContext())
            return ctx->GetActionMap().IsActionJustPressed(action);

        return false;
    }

    float InputSystem::GetAxis(const std::string& axis) {
        if (auto* ctx = GetActiveContext())
            return ctx->GetActionMap().GetAxis(axis);

        return 0.0f;
    }

    glm::vec2 InputSystem::GetAxis2D(const std::string& axis) {
        if (auto* ctx = GetActiveContext())
            return ctx->GetActionMap().GetAxis2D(axis);

        return glm::vec2(0.0f);
    }

}
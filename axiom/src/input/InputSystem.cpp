/**
 * @file axiom/src/Input/InputSystem.cpp
 * @brief Implements the input system module.
 */

#include "axiom/input/InputSystem.h"
#include "axiom/input/Input.h"

namespace axiom {

    std::unordered_map<std::string, std::shared_ptr<InputContext>>
        InputSystem::sRegisteredContexts;
    std::vector<std::shared_ptr<InputContext>> InputSystem::sContextStack;

    void InputSystem::init() {}

    void InputSystem::update() { Input::update(); }

    void
    InputSystem::registerContext(const std::shared_ptr<InputContext> &context) {
        sRegisteredContexts[context->getName()] = context;
    }

    void InputSystem::pushContext(const std::string &name) {
        auto it = sRegisteredContexts.find(name);
        if (it != sRegisteredContexts.end())
            sContextStack.push_back(it->second);
    }

    void InputSystem::popContext() {
        if (!sContextStack.empty())
            sContextStack.pop_back();
    }

    void InputSystem::clearContexts() { sContextStack.clear(); }

    InputContext *InputSystem::getActiveContext() {
        if (sContextStack.empty())
            return nullptr;

        return sContextStack.back().get();
    }

    // ==================== Wrappers ====================

    bool InputSystem::isActionPressed(const std::string &action) {
        if (auto *ctx = getActiveContext())
            return ctx->getActionMap().isActionPressed(action);

        return false;
    }

    bool InputSystem::isActionJustPressed(const std::string &action) {
        if (auto *ctx = getActiveContext())
            return ctx->getActionMap().isActionJustPressed(action);

        return false;
    }

    float InputSystem::getAxis(const std::string &axis) {
        if (auto *ctx = getActiveContext())
            return ctx->getActionMap().getAxis(axis);

        return 0.0f;
    }

    glm::vec2 InputSystem::getAxis2D(const std::string &axis) {
        if (auto *ctx = getActiveContext())
            return ctx->getActionMap().getAxis2D(axis);

        return glm::vec2(0.0f);
    }

} // namespace axiom
#include "axiom/input/ActionMap.h"
#include <glm/glm.hpp>
#include <cmath>

namespace axiom {

    // ================= BUTTONS =================

    void ActionMap::BindButton(const std::string& action, InputType type, int code) {
        m_ButtonBindings[action].push_back({ type, code });
    }

    void ActionMap::RebindButton(const std::string& action, size_t index, InputType type, int code) {
        m_ButtonBindings[action][index] = { type, code };
    }

    bool ActionMap::IsActionPressed(const std::string& action) const {
        auto it = m_ButtonBindings.find(action);
        if (it == m_ButtonBindings.end())
            return false;

        for (const auto& binding : it->second) {
            switch (binding.type) {
            case InputType::Key:
                if (Input::IsKeyPressed(binding.code)) return true;
                break;
            case InputType::MouseButton:
                if (Input::IsMousePressed(binding.code)) return true;
                break;
            case InputType::GamepadButton:
                if (Input::IsGamepadPressed(binding.code)) return true;
                break;
            default: break;
            }
        }

        return false;
    }

    bool ActionMap::IsActionJustPressed(const std::string& action) const {
        auto it = m_ButtonBindings.find(action);
        if (it == m_ButtonBindings.end())
            return false;

        for (const auto& binding : it->second) {
            switch (binding.type) {
            case InputType::Key:
                if (Input::IsKeyJustPressed(binding.code)) return true;
                break;
            case InputType::MouseButton:
                if (Input::IsMouseJustPressed(binding.code)) return true;
                break;
            default: break;
            }
        }

        return false;
    }

    // ================= AXIS =================

    void ActionMap::BindAxis(const std::string& axis, const AxisBinding& binding) {
        m_AxisBindings[axis].push_back(binding);
    }

    void ActionMap::RebindAxis(const std::string& axis, size_t index, const AxisBinding& binding) {
        m_AxisBindings[axis][index] = binding;
    }

    float ActionMap::GetAxis(const std::string& axis) const {
        auto it = m_AxisBindings.find(axis);
        if (it == m_AxisBindings.end())
            return 0.0f;

        float value = 0.0f;

        for (const auto& binding : it->second) {
            float input = 0.0f;

            switch (binding.type) {
            case InputType::Key:
                input = Input::IsKeyPressed(binding.code) ? 1.0f : 0.0f;
                break;

            case InputType::GamepadAxis:
                input = Input::GetGamepadAxis(binding.code);
                break;

            case InputType::MouseAxis:
            {
                glm::vec2 delta = Input::GetMouseDelta();
                input = (binding.code == 0) ? delta.x : delta.y;
                break;
            }

            default:
                break;
            }

            if (binding.invert)
                input = -input;

            if (std::abs(input) < binding.deadzone)
                input = 0.0f;

            value += input * binding.scale;
        }

        return glm::clamp(value, -1.0f, 1.0f);
    }

    // ================= AXIS 2D =================

    void ActionMap::BindAxis2D(const std::string& axis,
                               const std::string& xAxis,
                               const std::string& yAxis) {
        m_Axis2DBindings[axis] = { xAxis, yAxis };
    }

    glm::vec2 ActionMap::GetAxis2D(const std::string& axis) const {
        auto it = m_Axis2DBindings.find(axis);
        if (it == m_Axis2DBindings.end())
            return glm::vec2(0.0f);

        float x = GetAxis(it->second.xAxis);
        float y = GetAxis(it->second.yAxis);

        return glm::vec2(x, y);
    }

}
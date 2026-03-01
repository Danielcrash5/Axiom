#include "ActionMap.h"
#include <glm/gtc/clamp.hpp>

namespace axiom {

    // ================= Buttons =================

    void ActionMap::BindButton(const std::string& action, InputType type, int code) {
        m_ButtonBindings[action].push_back({ type, code });
    }

    bool ActionMap::IsActionPressed(const std::string& action) const {
        auto it = m_ButtonBindings.find(action);
        if (it == m_ButtonBindings.end())
            return false;

        for (const auto& binding : it->second) {
            switch (binding.type) {
            case InputType::Key:
                if (Input::IsKeyPressed(binding.code))
                    return true;
                break;

            case InputType::MouseButton:
                if (Input::IsMousePressed(binding.code))
                    return true;
                break;

            case InputType::GamepadButton:
                if (Input::IsGamepadPressed(binding.code))
                    return true;
                break;

            default:
                break;
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
                if (Input::IsKeyJustPressed(binding.code))
                    return true;
                break;

            case InputType::MouseButton:
                if (Input::IsMouseJustPressed(binding.code))
                    return true;
                break;

            case InputType::GamepadButton:
                if (Input::IsGamepadPressed(binding.code))
                    return true;
                break;

            default:
                break;
            }
        }

        return false;
    }

    // ================= Axis =================

    void ActionMap::BindAxisDigital(const std::string& axis, int negative, int positive) {
        AxisBinding binding;
        binding.type = AxisType::Digital;
        binding.negative = negative;
        binding.positive = positive;

        m_AxisBindings[axis].push_back(binding);
    }

    void ActionMap::BindAxisAnalog(const std::string& axis, int gamepadAxis, float scale) {
        AxisBinding binding;
        binding.type = AxisType::Analog;
        binding.axis = gamepadAxis;
        binding.scale = scale;

        m_AxisBindings[axis].push_back(binding);
    }

    float ActionMap::GetAxis(const std::string& axis) const {
        auto it = m_AxisBindings.find(axis);
        if (it == m_AxisBindings.end())
            return 0.0f;

        float value = 0.0f;

        for (const auto& binding : it->second) {
            if (binding.type == AxisType::Digital) {
                if (Input::IsKeyPressed(binding.negative))
                    value -= 1.0f;

                if (Input::IsKeyPressed(binding.positive))
                    value += 1.0f;
            }
            else if (binding.type == AxisType::Analog) {
                float axisValue = Input::GetGamepadAxis(binding.axis);
                value += axisValue * binding.scale;
            }
        }

        return glm::clamp(value, -1.0f, 1.0f);
    }

}
/**
 * @file axiom/src/Input/ActionMap.cpp
 * @brief Implements the action map module.
 */

#include "axiom/input/ActionMap.h"
#include <cmath>
#include <glm/glm.hpp>

namespace axiom {

    // ================= BUTTONS =================

    void ActionMap::bindButton(const std::string &action, InputType type,
                               int code, int deviceId) {
        m_buttonBindings[action].push_back({type, code, deviceId});
    }

    void ActionMap::bindHat(const std::string &action, InputType type,
                            JoystickHat mask, int hat, int deviceId) {
        m_buttonBindings[action].push_back(
            {type, static_cast<int>(mask), deviceId, hat});
    }

    void ActionMap::bindJoystickHat(const std::string &action, JoystickHat mask,
                                    int hat, int deviceId) {
        bindHat(action, InputType::joystickHat, mask, hat, deviceId);
    }

    void ActionMap::rebindButton(const std::string &action, size_t index,
                                 InputType type, int code, int deviceId) {
        m_buttonBindings[action][index] = {type, code, deviceId};
    }

    bool ActionMap::isActionPressed(const std::string &action) const {
        auto it = m_buttonBindings.find(action);
        if (it == m_buttonBindings.end())
            return false;

        for (const auto &binding : it->second) {
            switch (binding.type) {
            case InputType::key:
                if (Input::isKeyPressed(binding.code))
                    return true;
                break;
            case InputType::mouseButton:
                if (Input::isMousePressed(binding.code))
                    return true;
                break;
            case InputType::gamepadButton:
                if (Input::isGamepadPressed(binding.code, binding.deviceId))
                    return true;
                break;
            case InputType::joystickButton:
                if (Input::isJoystickButtonPressed(binding.code,
                                                   binding.deviceId))
                    return true;
                break;
            case InputType::joystickHat:
                if (Input::isJoystickHatPressed(
                        static_cast<JoystickHat>(binding.code), binding.index,
                        binding.deviceId))
                    return true;
                break;
            case InputType::customButton:
                if (Input::isCustomDeviceButtonPressed(binding.code,
                                                       binding.deviceId))
                    return true;
                break;
            case InputType::customHat:
                if (Input::isCustomDeviceHatPressed(
                        static_cast<JoystickHat>(binding.code), binding.index,
                        binding.deviceId))
                    return true;
                break;
            default:
                break;
            }
        }

        return false;
    }

    bool ActionMap::isActionJustPressed(const std::string &action) const {
        auto it = m_buttonBindings.find(action);
        if (it == m_buttonBindings.end())
            return false;

        for (const auto &binding : it->second) {
            switch (binding.type) {
            case InputType::key:
                if (Input::isKeyJustPressed(binding.code))
                    return true;
                break;
            case InputType::mouseButton:
                if (Input::isMouseJustPressed(binding.code))
                    return true;
                break;
            case InputType::gamepadButton:
                if (Input::isGamepadJustPressed(binding.code, binding.deviceId))
                    return true;
                break;
            case InputType::joystickButton:
                if (Input::isJoystickButtonJustPressed(binding.code,
                                                       binding.deviceId))
                    return true;
                break;
            case InputType::joystickHat:
                if (Input::isJoystickHatJustPressed(
                        static_cast<JoystickHat>(binding.code), binding.index,
                        binding.deviceId))
                    return true;
                break;
            case InputType::customButton:
                if (Input::isCustomDeviceButtonJustPressed(binding.code,
                                                           binding.deviceId))
                    return true;
                break;
            case InputType::customHat:
                if (Input::isCustomDeviceHatJustPressed(
                        static_cast<JoystickHat>(binding.code), binding.index,
                        binding.deviceId))
                    return true;
                break;
            default:
                break;
            }
        }

        return false;
    }

    // ================= AXIS =================

    void ActionMap::bindAxis(const std::string &axis,
                             const AxisBinding &binding) {
        m_axisBindings[axis].push_back(binding);
    }

    void ActionMap::rebindAxis(const std::string &axis, size_t index,
                               const AxisBinding &binding) {
        m_axisBindings[axis][index] = binding;
    }

    float ActionMap::getAxis(const std::string &axis) const {
        auto it = m_axisBindings.find(axis);
        if (it == m_axisBindings.end())
            return 0.0f;

        float value = 0.0f;

        for (const auto &binding : it->second) {
            float input = 0.0f;

            switch (binding.type) {
            case InputType::key:
                input = Input::isKeyPressed(binding.code) ? 1.0f : 0.0f;
                break;

            case InputType::gamepadAxis:
                input = Input::getGamepadAxis(binding.code, binding.deviceId);
                break;

            case InputType::joystickAxis:
                input = Input::getJoystickAxis(binding.code, binding.deviceId,
                                               binding.deadzone);
                break;

            case InputType::customAxis:
                input = Input::getCustomDeviceAxis(
                    binding.code, binding.deviceId, binding.deadzone);
                break;

            case InputType::mouseAxis: {
                glm::vec2 delta = Input::getMouseDelta();
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

    void ActionMap::bindAxis2D(const std::string &axis,
                               const std::string &xAxis,
                               const std::string &yAxis) {
        m_axis2DBindings[axis] = {xAxis, yAxis};
    }

    glm::vec2 ActionMap::getAxis2D(const std::string &axis) const {
        auto it = m_axis2DBindings.find(axis);
        if (it == m_axis2DBindings.end())
            return glm::vec2(0.0f);

        float x = getAxis(it->second.xAxis);
        float y = getAxis(it->second.yAxis);

        return glm::vec2(x, y);
    }

} // namespace axiom

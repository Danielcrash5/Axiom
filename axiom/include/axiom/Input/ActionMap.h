#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <glm/glm.hpp>
#include "Input.h"

namespace axiom {

    enum class InputType {
        Key,
        MouseButton,
        GamepadButton,
        GamepadAxis
    };

    // ---------------------
    // Button Binding
    // ---------------------

    struct ButtonBinding {
        InputType type;
        int code;
    };

    // ---------------------
    // Axis Binding
    // ---------------------

    enum class AxisType {
        Digital,   // two buttons (negative / positive)
        Analog     // single axis (gamepad)
    };

    struct AxisBinding {
        AxisType type;

        // Digital
        int negative = 0;
        int positive = 0;

        // Analog
        int axis = 0;
        float scale = 1.0f;
    };

    // =====================
    // ActionMap
    // =====================

    class ActionMap {
    public:

        // -------- Buttons --------
        void BindButton(const std::string& action, InputType type, int code);
        bool IsActionPressed(const std::string& action) const;
        bool IsActionJustPressed(const std::string& action) const;

        // -------- Axis --------
        void BindAxisDigital(const std::string& axis, int negative, int positive);
        void BindAxisAnalog(const std::string& axis, int gamepadAxis, float scale = 1.0f);

        float GetAxis(const std::string& axis) const;

    private:
        std::unordered_map<std::string, std::vector<ButtonBinding>> m_ButtonBindings;
        std::unordered_map<std::string, std::vector<AxisBinding>> m_AxisBindings;
    };

}
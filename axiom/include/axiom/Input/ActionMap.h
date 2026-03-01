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
        GamepadAxis,
        MouseAxis
    };

    // ======================
    // Button Binding
    // ======================

    struct ButtonBinding {
        InputType type;
        int code;
    };

    // ======================
    // Axis Binding
    // ======================

    struct AxisBinding {
        InputType type;
        int code;

        float scale = 1.0f;
        float deadzone = 0.0f;

        bool invert = false;
    };

    // ======================
    // ActionMap
    // ======================

    class ActionMap {
    public:

        // ---- Buttons ----
        void BindButton(const std::string& action, InputType type, int code);
        bool IsActionPressed(const std::string& action) const;
        bool IsActionJustPressed(const std::string& action) const;

        void RebindButton(const std::string& action, size_t index, InputType type, int code);

        // ---- Axis 1D ----
        void BindAxis(const std::string& axis, const AxisBinding& binding);
        float GetAxis(const std::string& axis) const;

        void RebindAxis(const std::string& axis, size_t index, const AxisBinding& binding);

        // ---- Axis 2D ----
        void BindAxis2D(const std::string& axis,
                        const std::string& xAxis,
                        const std::string& yAxis);

        glm::vec2 GetAxis2D(const std::string& axis) const;

    private:

        std::unordered_map<std::string, std::vector<ButtonBinding>> m_ButtonBindings;
        std::unordered_map<std::string, std::vector<AxisBinding>>   m_AxisBindings;

        struct Axis2D {
            std::string xAxis;
            std::string yAxis;
        };

        std::unordered_map<std::string, Axis2D> m_Axis2DBindings;
    };

}
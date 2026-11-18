/**
 * @file axiom/include/axiom/Input/ActionMap.h
 * @brief Declares the public interface for the action map.
 */

#pragma once

#include "Input.h"
#include <glm/glm.hpp>
#include <string>
#include <unordered_map>
#include <vector>

namespace axiom {

    /**
     * @brief Defines the available input type values.
     * @tparam InputType Template type used by this type.
     */
    enum class InputType {
        /** @brief Represents the key value. */
        key,
        /** @brief Represents the mouse button value. */
        mouseButton,
        /** @brief Represents the gamepad button value. */
        gamepadButton,
        /** @brief Represents the gamepad axis value. */
        gamepadAxis,
        /** @brief Represents the joystick button value. */
        joystickButton,
        /** @brief Represents the joystick axis value. */
        joystickAxis,
        /** @brief Represents the joystick hat value. */
        joystickHat,
        /** @brief Represents the custom button value. */
        customButton,
        /** @brief Represents the custom axis value. */
        customAxis,
        /** @brief Represents the custom hat value. */
        customHat,
        mouseAxis
    };

    // ======================
    // Button Binding
    // ======================

    /** @brief Stores the data for the button binding. */
    struct ButtonBinding {
        /** @brief Stores the type. */
        InputType type;
        /** @brief Stores the code. */
        int code;
        /** @brief Stores the device ID. */
        int deviceId = 0;
        /** @brief Stores the index. */
        int index = 0;
    };

    // ======================
    // Axis Binding
    // ======================

    /** @brief Stores the data for the axis binding. */
    struct AxisBinding {
        /** @brief Stores the type. */
        InputType type;
        /** @brief Stores the code. */
        int code;
        /** @brief Stores the device ID. */
        int deviceId = 0;

        /** @brief Stores the scale. */
        float scale = 1.0f;
        /** @brief Stores the deadzone. */
        float deadzone = 0.0f;

        /** @brief Stores the invert. */
        bool invert = false;
    };

    // ======================
    // ActionMap
    // ======================

    /**
     * @brief Maps named actions and axes to physical input bindings.
     * @tparam ActionMap Template type used by this type.
     */
    class ActionMap {
      public:
        // ---- Buttons ----
        /**
         * @brief Binds button.
         * @param action The action used by this operation.
         * @param type The type used by this operation.
         * @param code The code used by this operation.
         * @param deviceId The device ID used by this operation.
         */
        void bindButton(const std::string &action, InputType type, int code,
                        int deviceId = 0);
        /**
         * @brief Binds hat.
         * @param action The action used by this operation.
         * @param type The type used by this operation.
         * @param mask The mask used by this operation.
         * @param hat The hat used by this operation.
         * @param deviceId The device ID used by this operation.
         */
        void bindHat(const std::string &action, InputType type,
                     JoystickHat mask, int hat = 0, int deviceId = 0);
        /**
         * @brief Binds joystick hat.
         * @param action The action used by this operation.
         * @param mask The mask used by this operation.
         * @param hat The hat used by this operation.
         * @param deviceId The device ID used by this operation.
         */
        void bindJoystickHat(const std::string &action, JoystickHat mask,
                             int hat = 0, int deviceId = 0);
        /**
         * @brief Reports whether action pressed.
         * @param action The action used by this operation.
         * @return The result of the operation.
         */
        bool isActionPressed(const std::string &action) const;
        /**
         * @brief Reports whether action just pressed.
         * @param action The action used by this operation.
         * @return The result of the operation.
         */
        bool isActionJustPressed(const std::string &action) const;

        /**
         * @brief Performs the rebind button operation.
         * @param action The action used by this operation.
         * @param index The index used by this operation.
         * @param type The type used by this operation.
         * @param code The code used by this operation.
         * @param deviceId The device ID used by this operation.
         */
        void rebindButton(const std::string &action, size_t index,
                          InputType type, int code, int deviceId = 0);

        // ---- Axis 1D ----
        /**
         * @brief Binds axis.
         * @param axis The axis used by this operation.
         * @param binding The binding used by this operation.
         */
        void bindAxis(const std::string &axis, const AxisBinding &binding);
        /**
         * @brief Returns the axis.
         * @param axis The axis used by this operation.
         * @return The result of the operation.
         */
        float getAxis(const std::string &axis) const;

        /**
         * @brief Performs the rebind axis operation.
         * @param axis The axis used by this operation.
         * @param index The index used by this operation.
         * @param binding The binding used by this operation.
         */
        void rebindAxis(const std::string &axis, size_t index,
                        const AxisBinding &binding);

        // ---- Axis 2D ----
        /**
         * @brief Binds axis2 d.
         * @param axis The axis used by this operation.
         * @param xAxis The x axis used by this operation.
         * @param yAxis The y axis used by this operation.
         */
        void bindAxis2D(const std::string &axis, const std::string &xAxis,
                        const std::string &yAxis);

        /**
         * @brief Returns the axis2 d.
         * @param axis The axis used by this operation.
         * @return The result of the operation.
         */
        glm::vec2 getAxis2D(const std::string &axis) const;

      private:
        std::unordered_map<std::string, std::vector<ButtonBinding>>
            /** @brief Stores the button bindings. */
            m_buttonBindings;
        std::unordered_map<std::string, std::vector<AxisBinding>>
            /** @brief Stores the axis bindings. */
            m_axisBindings;

        /** @brief Stores the data for the axis2 d. */
        struct Axis2D {
            /** @brief Stores the x axis. */
            std::string xAxis;
            /** @brief Stores the y axis. */
            std::string yAxis;
        };

        /** @brief Stores the axis2 d bindings. */
        std::unordered_map<std::string, Axis2D> m_axis2DBindings;
    };

} // namespace axiom

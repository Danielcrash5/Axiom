/**
 * @file axiom/include/axiom/Input/Input.h
 * @brief Declares the public interface for the input.
 */

#pragma once

#include <glm/glm.hpp>

#include <array>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "CustomInputDevice.h"
#include "GamepadCodes.h"
#include "JoystickCodes.h"
#include "KeyCodes.h"
#include "MouseCodes.h"

struct SDL_Window;
struct SDL_Gamepad;
struct SDL_Joystick;

namespace axiom {

    /**
     * @brief Provides the input functionality.
     * @tparam Input Template type used by this type.
     */
    class Input {
      public:
        /**
         * @brief Initializes the subsystem.
         * @param window The window used by this operation.
         */
        static void init(void *window);
        /**
         * @brief Shuts down the subsystem.
         */
        static void shutdown();
        /**
         * @brief Updates the object state.
         */
        static void update();

        // ================= Keyboard =================
        /**
         * @brief Reports whether key pressed.
         * @param key The key used by this operation.
         * @return The result of the operation.
         */
        static bool isKeyPressed(KeyCode key);
        /**
         * @brief Reports whether key just pressed.
         * @param key The key used by this operation.
         * @return The result of the operation.
         */
        static bool isKeyJustPressed(KeyCode key);
        /**
         * @brief Reports whether key released.
         * @param key The key used by this operation.
         * @return The result of the operation.
         */
        static bool isKeyReleased(KeyCode key);

        // ================= Mouse =================
        /**
         * @brief Reports whether mouse pressed.
         * @param button The button used by this operation.
         * @return The result of the operation.
         */
        static bool isMousePressed(MouseCode button);
        /**
         * @brief Reports whether mouse just pressed.
         * @param button The button used by this operation.
         * @return The result of the operation.
         */
        static bool isMouseJustPressed(MouseCode button);
        /**
         * @brief Reports whether mouse released.
         * @param button The button used by this operation.
         * @return The result of the operation.
         */
        static bool isMouseReleased(MouseCode button);

        /**
         * @brief Returns the mouse position.
         * @return The result of the operation.
         */
        static glm::vec2 getMousePosition();
        /**
         * @brief Returns the mouse delta.
         * @return The result of the operation.
         */
        static glm::vec2 getMouseDelta();
        /**
         * @brief Returns the mouse scroll.
         * @return The result of the operation.
         */
        static glm::vec2 getMouseScroll();
        /**
         * @brief Returns the mouse scroll delta.
         * @return The result of the operation.
         */
        static glm::vec2 getMouseScrollDelta();

        // ================= Gamepad =================
        /**
         * @brief Reports whether gamepad connected.
         * @param id The ID used by this operation.
         * @return The result of the operation.
         */
        static bool isGamepadConnected(int id = 0);
        /**
         * @brief Returns the gamepad name.
         * @param id The ID used by this operation.
         * @return The result of the operation.
         */
        static std::string getGamepadName(int id = 0);

        /**
         * @brief Reports whether gamepad pressed.
         * @param button The button used by this operation.
         * @param id The ID used by this operation.
         * @return The result of the operation.
         */
        static bool isGamepadPressed(GamepadButton button, int id = 0);
        /**
         * @brief Reports whether gamepad just pressed.
         * @param button The button used by this operation.
         * @param id The ID used by this operation.
         * @return The result of the operation.
         */
        static bool isGamepadJustPressed(GamepadButton button, int id = 0);
        /**
         * @brief Reports whether gamepad released.
         * @param button The button used by this operation.
         * @param id The ID used by this operation.
         * @return The result of the operation.
         */
        static bool isGamepadReleased(GamepadButton button, int id = 0);

        /**
         * @brief Returns the gamepad left stick.
         * @param id The ID used by this operation.
         * @return The result of the operation.
         */
        static glm::vec2 getGamepadLeftStick(int id = 0);
        /**
         * @brief Returns the gamepad right stick.
         * @param id The ID used by this operation.
         * @return The result of the operation.
         */
        static glm::vec2 getGamepadRightStick(int id = 0);

        /**
         * @brief Returns the gamepad axis.
         * @param axis The axis used by this operation.
         * @param id The ID used by this operation.
         * @return The result of the operation.
         */
        static float getGamepadAxis(GamepadAxis axis, int id = 0);

        /**
         * @brief Returns the gamepad left trigger.
         * @param id The ID used by this operation.
         * @return The result of the operation.
         */
        static float getGamepadLeftTrigger(int id = 0);
        /**
         * @brief Returns the gamepad right trigger.
         * @param id The ID used by this operation.
         * @return The result of the operation.
         */
        static float getGamepadRightTrigger(int id = 0);

        /**
         * @brief Performs the rumble gamepad operation.
         * @param lowFrequency The low frequency used by this operation.
         * @param highFrequency The high frequency used by this operation.
         * @param durationMs The duration ms used by this operation.
         * @param id The ID used by this operation.
         * @return The result of the operation.
         */
        static bool rumbleGamepad(uint16_t lowFrequency, uint16_t highFrequency,
                                  uint32_t durationMs, int id = 0);
        /**
         * @brief Performs the rumble gamepad triggers operation.
         * @param leftTrigger The left trigger used by this operation.
         * @param rightTrigger The right trigger used by this operation.
         * @param durationMs The duration ms used by this operation.
         * @param id The ID used by this operation.
         * @return The result of the operation.
         */
        static bool rumbleGamepadTriggers(uint16_t leftTrigger,
                                          uint16_t rightTrigger,
                                          uint32_t durationMs, int id = 0);
        /**
         * @brief Sets gamepad led.
         * @param red The red used by this operation.
         * @param green The green used by this operation.
         * @param blue The blue used by this operation.
         * @param id The ID used by this operation.
         * @return The result of the operation.
         */
        static bool setGamepadLed(uint8_t red, uint8_t green, uint8_t blue,
                                  int id = 0);
        /**
         * @brief Sets gamepad sensor enabled.
         * @param sensor The sensor used by this operation.
         * @param enabled The enabled used by this operation.
         * @param id The ID used by this operation.
         * @return The result of the operation.
         */
        static bool setGamepadSensorEnabled(GamepadSensor sensor, bool enabled,
                                            int id = 0);
        /**
         * @brief Returns the gamepad sensor data.
         * @param sensor The sensor used by this operation.
         * @param data The data used by this operation.
         * @param valueCount The value count used by this operation.
         * @param id The ID used by this operation.
         * @return The result of the operation.
         */
        static bool getGamepadSensorData(GamepadSensor sensor, float *data,
                                         int valueCount, int id = 0);
        /**
         * @brief Returns the gamepad touchpad count.
         * @param id The ID used by this operation.
         * @return The result of the operation.
         */
        static int getGamepadTouchpadCount(int id = 0);
        /**
         * @brief Returns the gamepad touchpad finger count.
         * @param touchpad The touchpad used by this operation.
         * @param id The ID used by this operation.
         * @return The result of the operation.
         */
        static int getGamepadTouchpadFingerCount(int touchpad = 0, int id = 0);
        /**
         * @brief Returns the gamepad touchpad finger.
         * @param touchpad The touchpad used by this operation.
         * @param finger The finger used by this operation.
         * @param down The down used by this operation.
         * @param x The x used by this operation.
         * @param y The y used by this operation.
         * @param pressure The pressure used by this operation.
         * @param id The ID used by this operation.
         * @return The result of the operation.
         */
        static bool getGamepadTouchpadFinger(int touchpad, int finger,
                                             bool &down, float &x, float &y,
                                             float &pressure, int id = 0);

        // ================= Joystick =================
        /**
         * @brief Reports whether joystick connected.
         * @param id The ID used by this operation.
         * @return The result of the operation.
         */
        static bool isJoystickConnected(int id = 0);
        /**
         * @brief Returns the joystick name.
         * @param id The ID used by this operation.
         * @return The result of the operation.
         */
        static std::string getJoystickName(int id = 0);
        /**
         * @brief Returns the joystick count.
         * @return The result of the operation.
         */
        static int getJoystickCount();
        /**
         * @brief Returns the joystick axis count.
         * @param id The ID used by this operation.
         * @return The result of the operation.
         */
        static int getJoystickAxisCount(int id = 0);
        /**
         * @brief Returns the joystick button count.
         * @param id The ID used by this operation.
         * @return The result of the operation.
         */
        static int getJoystickButtonCount(int id = 0);
        /**
         * @brief Returns the joystick hat count.
         * @param id The ID used by this operation.
         * @return The result of the operation.
         */
        static int getJoystickHatCount(int id = 0);

        /**
         * @brief Returns the joystick axis.
         * @param axis The axis used by this operation.
         * @param id The ID used by this operation.
         * @param deadzone The deadzone used by this operation.
         * @return The result of the operation.
         */
        static float getJoystickAxis(JoystickAxis axis, int id = 0,
                                     float deadzone = 0.1f);
        /**
         * @brief Returns the joystick axis raw.
         * @param axis The axis used by this operation.
         * @param id The ID used by this operation.
         * @return The result of the operation.
         */
        static int16_t getJoystickAxisRaw(JoystickAxis axis, int id = 0);

        /**
         * @brief Reports whether joystick button pressed.
         * @param button The button used by this operation.
         * @param id The ID used by this operation.
         * @return The result of the operation.
         */
        static bool isJoystickButtonPressed(JoystickButton button, int id = 0);
        /**
         * @brief Reports whether joystick button just pressed.
         * @param button The button used by this operation.
         * @param id The ID used by this operation.
         * @return The result of the operation.
         */
        static bool isJoystickButtonJustPressed(JoystickButton button,
                                                int id = 0);
        /**
         * @brief Reports whether joystick button released.
         * @param button The button used by this operation.
         * @param id The ID used by this operation.
         * @return The result of the operation.
         */
        static bool isJoystickButtonReleased(JoystickButton button, int id = 0);

        /**
         * @brief Returns the joystick hat.
         * @param hat The hat used by this operation.
         * @param id The ID used by this operation.
         * @return The result of the operation.
         */
        static JoystickHat getJoystickHat(int hat = 0, int id = 0);
        /**
         * @brief Reports whether joystick hat pressed.
         * @param mask The mask used by this operation.
         * @param hat The hat used by this operation.
         * @param id The ID used by this operation.
         * @return The result of the operation.
         */
        static bool isJoystickHatPressed(JoystickHat mask, int hat = 0,
                                         int id = 0);
        /**
         * @brief Reports whether joystick hat just pressed.
         * @param mask The mask used by this operation.
         * @param hat The hat used by this operation.
         * @param id The ID used by this operation.
         * @return The result of the operation.
         */
        static bool isJoystickHatJustPressed(JoystickHat mask, int hat = 0,
                                             int id = 0);
        /**
         * @brief Reports whether joystick hat released.
         * @param mask The mask used by this operation.
         * @param hat The hat used by this operation.
         * @param id The ID used by this operation.
         * @return The result of the operation.
         */
        static bool isJoystickHatReleased(JoystickHat mask, int hat = 0,
                                          int id = 0);

        /**
         * @brief Performs the rumble joystick operation.
         * @param lowFrequency The low frequency used by this operation.
         * @param highFrequency The high frequency used by this operation.
         * @param durationMs The duration ms used by this operation.
         * @param id The ID used by this operation.
         * @return The result of the operation.
         */
        static bool rumbleJoystick(uint16_t lowFrequency,
                                   uint16_t highFrequency, uint32_t durationMs,
                                   int id = 0);
        /**
         * @brief Performs the rumble joystick triggers operation.
         * @param leftTrigger The left trigger used by this operation.
         * @param rightTrigger The right trigger used by this operation.
         * @param durationMs The duration ms used by this operation.
         * @param id The ID used by this operation.
         * @return The result of the operation.
         */
        static bool rumbleJoystickTriggers(uint16_t leftTrigger,
                                           uint16_t rightTrigger,
                                           uint32_t durationMs, int id = 0);
        /**
         * @brief Sets joystick led.
         * @param red The red used by this operation.
         * @param green The green used by this operation.
         * @param blue The blue used by this operation.
         * @param id The ID used by this operation.
         * @return The result of the operation.
         */
        static bool setJoystickLed(uint8_t red, uint8_t green, uint8_t blue,
                                   int id = 0);

        // ================= Custom Devices =================
        /**
         * @brief Registers custom device.
         * @param device The device used by this operation.
         * @return The result of the operation.
         */
        static int
        registerCustomDevice(std::shared_ptr<ICustomInputDevice> device);
        /**
         * @brief Unregisters custom device.
         * @param id The ID used by this operation.
         */
        static void unregisterCustomDevice(int id);
        /**
         * @brief Clears custom devices.
         */
        static void clearCustomDevices();

        /**
         * @brief Reports whether custom device connected.
         * @param id The ID used by this operation.
         * @return The result of the operation.
         */
        static bool isCustomDeviceConnected(int id = 0);
        /**
         * @brief Returns the custom device name.
         * @param id The ID used by this operation.
         * @return The result of the operation.
         */
        static std::string getCustomDeviceName(int id = 0);
        /**
         * @brief Returns the custom device count.
         * @return The result of the operation.
         */
        static int getCustomDeviceCount();
        /**
         * @brief Returns the custom device axis count.
         * @param id The ID used by this operation.
         * @return The result of the operation.
         */
        static int getCustomDeviceAxisCount(int id = 0);
        /**
         * @brief Returns the custom device button count.
         * @param id The ID used by this operation.
         * @return The result of the operation.
         */
        static int getCustomDeviceButtonCount(int id = 0);
        /**
         * @brief Returns the custom device hat count.
         * @param id The ID used by this operation.
         * @return The result of the operation.
         */
        static int getCustomDeviceHatCount(int id = 0);

        /**
         * @brief Returns the custom device axis.
         * @param axis The axis used by this operation.
         * @param id The ID used by this operation.
         * @param deadzone The deadzone used by this operation.
         * @return The result of the operation.
         */
        static float getCustomDeviceAxis(int axis, int id = 0,
                                         float deadzone = 0.0f);
        /**
         * @brief Reports whether custom device button pressed.
         * @param button The button used by this operation.
         * @param id The ID used by this operation.
         * @return The result of the operation.
         */
        static bool isCustomDeviceButtonPressed(int button, int id = 0);
        /**
         * @brief Reports whether custom device button just pressed.
         * @param button The button used by this operation.
         * @param id The ID used by this operation.
         * @return The result of the operation.
         */
        static bool isCustomDeviceButtonJustPressed(int button, int id = 0);
        /**
         * @brief Reports whether custom device button released.
         * @param button The button used by this operation.
         * @param id The ID used by this operation.
         * @return The result of the operation.
         */
        static bool isCustomDeviceButtonReleased(int button, int id = 0);
        /**
         * @brief Returns the custom device hat.
         * @param hat The hat used by this operation.
         * @param id The ID used by this operation.
         * @return The result of the operation.
         */
        static JoystickHat getCustomDeviceHat(int hat = 0, int id = 0);
        /**
         * @brief Reports whether custom device hat pressed.
         * @param mask The mask used by this operation.
         * @param hat The hat used by this operation.
         * @param id The ID used by this operation.
         * @return The result of the operation.
         */
        static bool isCustomDeviceHatPressed(JoystickHat mask, int hat = 0,
                                             int id = 0);
        /**
         * @brief Reports whether custom device hat just pressed.
         * @param mask The mask used by this operation.
         * @param hat The hat used by this operation.
         * @param id The ID used by this operation.
         * @return The result of the operation.
         */
        static bool isCustomDeviceHatJustPressed(JoystickHat mask, int hat = 0,
                                                 int id = 0);
        /**
         * @brief Reports whether custom device hat released.
         * @param mask The mask used by this operation.
         * @param hat The hat used by this operation.
         * @param id The ID used by this operation.
         * @return The result of the operation.
         */
        static bool isCustomDeviceHatReleased(JoystickHat mask, int hat = 0,
                                              int id = 0);

      private:
        /** @brief Stores the data for the joystick device. */
        struct JoystickDevice {
            /** @brief Stores the handle. */
            SDL_Joystick *Handle = nullptr;
            /** @brief Stores the instance ID. */
            SDL_JoystickID InstanceID = 0;
            /** @brief Stores the name. */
            std::string Name;
            /** @brief Stores the axes. */
            std::vector<float> Axes;
            /** @brief Stores the raw axes. */
            std::vector<int16_t> RawAxes;
            /** @brief Stores the current buttons. */
            std::vector<bool> CurrentButtons;
            /** @brief Stores the previous buttons. */
            std::vector<bool> PreviousButtons;
            /** @brief Stores the hats. */
            std::vector<JoystickHat> Hats;
            /** @brief Stores the previous hats. */
            std::vector<JoystickHat> PreviousHats;
        };

        /** @brief Stores the window. */
        static SDL_Window *sWindow;

        /** @brief Stores the current keys. */
        static std::array<bool, SDL_SCANCODE_COUNT> sCurrentKeys;
        /** @brief Stores the previous keys. */
        static std::array<bool, SDL_SCANCODE_COUNT> sPreviousKeys;

        /** @brief Stores the current mouse. */
        static std::array<bool, 9> sCurrentMouse;
        /** @brief Stores the previous mouse. */
        static std::array<bool, 9> sPreviousMouse;

        /** @brief Stores the gamepads. */
        static std::array<SDL_Gamepad *, 4> sGamepads;
        static std::array<std::array<bool, SDL_GAMEPAD_BUTTON_COUNT>, 4>
            /** @brief Stores the current gamepad buttons. */
            sCurrentGamepadButtons;
        static std::array<std::array<bool, SDL_GAMEPAD_BUTTON_COUNT>, 4>
            /** @brief Stores the previous gamepad buttons. */
            sPreviousGamepadButtons;
        /** @brief Stores the joysticks. */
        static std::array<JoystickDevice, 8> sJoysticks;
        /** @brief Stores the joystick count. */
        static int sJoystickCount;
        /** @brief Stores the custom devices. */
        static std::vector<std::shared_ptr<ICustomInputDevice>> sCustomDevices;

        /** @brief Stores the mouse position. */
        static glm::vec2 sMousePosition;
        /** @brief Stores the previous mouse position. */
        static glm::vec2 sPreviousMousePosition;
        /** @brief Stores the mouse scroll. */
        static glm::vec2 sMouseScroll;
        /** @brief Stores the pending mouse scroll. */
        static glm::vec2 sPendingMouseScroll;

        /**
         * @brief Performs the apply deadzone operation.
         * @param value The value used by this operation.
         * @param deadzone The deadzone used by this operation.
         * @return The result of the operation.
         */
        static float applyDeadzone(float value, float deadzone = 0.1f);
        /**
         * @brief Reports whether valid joystick slot.
         * @param id The ID used by this operation.
         * @return The result of the operation.
         */
        static bool isValidJoystickSlot(int id);
        /**
         * @brief Returns the gamepad.
         * @param id The ID used by this operation.
         * @return The result of the operation.
         */
        static SDL_Gamepad *getGamepad(int id);
        /**
         * @brief Returns the joystick.
         * @param id The ID used by this operation.
         * @return The result of the operation.
         */
        static JoystickDevice *getJoystick(int id);
        /**
         * @brief Performs the refresh gamepads operation.
         */
        static void refreshGamepads();
        /**
         * @brief Performs the refresh joysticks operation.
         */
        static void refreshJoysticks();
        /**
         * @brief Updates joysticks.
         */
        static void updateJoysticks();

      public:
        // Internal hooks used by the window backend.
        /**
         * @brief Handles the mouse scroll callback.
         * @param xOffset The x offset used by this operation.
         * @param yOffset The y offset used by this operation.
         */
        static void onMouseScroll(double xOffset, double yOffset);
        /**
         * @brief Handles the gamepad changed callback.
         */
        static void onGamepadChanged();
        /**
         * @brief Handles the joystick changed callback.
         */
        static void onJoystickChanged();
    };

} // namespace axiom

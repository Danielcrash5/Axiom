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

class Input {
  public:
    static void Init(void *window);
    static void Shutdown();
    static void Update();

    // ================= Keyboard =================
    static bool IsKeyPressed(KeyCode key);
    static bool IsKeyJustPressed(KeyCode key);
    static bool IsKeyReleased(KeyCode key);

    // ================= Mouse =================
    static bool IsMousePressed(MouseCode button);
    static bool IsMouseJustPressed(MouseCode button);
    static bool IsMouseReleased(MouseCode button);

    static glm::vec2 GetMousePosition();
    static glm::vec2 GetMouseDelta();
    static glm::vec2 GetMouseScroll();
    static glm::vec2 GetMouseScrollDelta();

    // ================= Gamepad =================
    static bool IsGamepadConnected(int id = 0);
    static std::string GetGamepadName(int id = 0);

    static bool IsGamepadPressed(GamepadButton button, int id = 0);
    static bool IsGamepadJustPressed(GamepadButton button, int id = 0);
    static bool IsGamepadReleased(GamepadButton button, int id = 0);

    static glm::vec2 GetGamepadLeftStick(int id = 0);
    static glm::vec2 GetGamepadRightStick(int id = 0);

    static float GetGamepadAxis(GamepadAxis axis, int id = 0);

    static float GetGamepadLeftTrigger(int id = 0);
    static float GetGamepadRightTrigger(int id = 0);

    static bool RumbleGamepad(uint16_t lowFrequency, uint16_t highFrequency,
                              uint32_t durationMs, int id = 0);
    static bool RumbleGamepadTriggers(uint16_t leftTrigger,
                                      uint16_t rightTrigger,
                                      uint32_t durationMs, int id = 0);
    static bool SetGamepadLED(uint8_t red, uint8_t green, uint8_t blue,
                              int id = 0);
    static bool SetGamepadSensorEnabled(GamepadSensor sensor, bool enabled,
                                        int id = 0);
    static bool GetGamepadSensorData(GamepadSensor sensor, float *data,
                                     int valueCount, int id = 0);
    static int GetGamepadTouchpadCount(int id = 0);
    static int GetGamepadTouchpadFingerCount(int touchpad = 0, int id = 0);
    static bool GetGamepadTouchpadFinger(int touchpad, int finger, bool &down,
                                         float &x, float &y, float &pressure,
                                         int id = 0);

    // ================= Joystick =================
    static bool IsJoystickConnected(int id = 0);
    static std::string GetJoystickName(int id = 0);
    static int GetJoystickCount();
    static int GetJoystickAxisCount(int id = 0);
    static int GetJoystickButtonCount(int id = 0);
    static int GetJoystickHatCount(int id = 0);

    static float GetJoystickAxis(JoystickAxis axis, int id = 0,
                                 float deadzone = 0.1f);
    static int16_t GetJoystickAxisRaw(JoystickAxis axis, int id = 0);

    static bool IsJoystickButtonPressed(JoystickButton button, int id = 0);
    static bool IsJoystickButtonJustPressed(JoystickButton button, int id = 0);
    static bool IsJoystickButtonReleased(JoystickButton button, int id = 0);

    static JoystickHat GetJoystickHat(int hat = 0, int id = 0);
    static bool IsJoystickHatPressed(JoystickHat mask, int hat = 0, int id = 0);
    static bool IsJoystickHatJustPressed(JoystickHat mask, int hat = 0,
                                         int id = 0);
    static bool IsJoystickHatReleased(JoystickHat mask, int hat = 0,
                                      int id = 0);

    static bool RumbleJoystick(uint16_t lowFrequency, uint16_t highFrequency,
                               uint32_t durationMs, int id = 0);
    static bool RumbleJoystickTriggers(uint16_t leftTrigger,
                                       uint16_t rightTrigger,
                                       uint32_t durationMs, int id = 0);
    static bool SetJoystickLED(uint8_t red, uint8_t green, uint8_t blue,
                               int id = 0);

    // ================= Custom Devices =================
    static int RegisterCustomDevice(std::shared_ptr<ICustomInputDevice> device);
    static void UnregisterCustomDevice(int id);
    static void ClearCustomDevices();

    static bool IsCustomDeviceConnected(int id = 0);
    static std::string GetCustomDeviceName(int id = 0);
    static int GetCustomDeviceCount();
    static int GetCustomDeviceAxisCount(int id = 0);
    static int GetCustomDeviceButtonCount(int id = 0);
    static int GetCustomDeviceHatCount(int id = 0);

    static float GetCustomDeviceAxis(int axis, int id = 0,
                                     float deadzone = 0.0f);
    static bool IsCustomDeviceButtonPressed(int button, int id = 0);
    static bool IsCustomDeviceButtonJustPressed(int button, int id = 0);
    static bool IsCustomDeviceButtonReleased(int button, int id = 0);
    static JoystickHat GetCustomDeviceHat(int hat = 0, int id = 0);
    static bool IsCustomDeviceHatPressed(JoystickHat mask, int hat = 0,
                                         int id = 0);
    static bool IsCustomDeviceHatJustPressed(JoystickHat mask, int hat = 0,
                                             int id = 0);
    static bool IsCustomDeviceHatReleased(JoystickHat mask, int hat = 0,
                                          int id = 0);

  private:
    struct JoystickDevice {
        SDL_Joystick *Handle = nullptr;
        SDL_JoystickID InstanceID = 0;
        std::string Name;
        std::vector<float> Axes;
        std::vector<int16_t> RawAxes;
        std::vector<bool> CurrentButtons;
        std::vector<bool> PreviousButtons;
        std::vector<JoystickHat> Hats;
        std::vector<JoystickHat> PreviousHats;
    };

    static SDL_Window *s_Window;

    static std::array<bool, SDL_SCANCODE_COUNT> s_CurrentKeys;
    static std::array<bool, SDL_SCANCODE_COUNT> s_PreviousKeys;

    static std::array<bool, 9> s_CurrentMouse;
    static std::array<bool, 9> s_PreviousMouse;

    static std::array<SDL_Gamepad *, 4> s_Gamepads;
    static std::array<std::array<bool, SDL_GAMEPAD_BUTTON_COUNT>, 4>
        s_CurrentGamepadButtons;
    static std::array<std::array<bool, SDL_GAMEPAD_BUTTON_COUNT>, 4>
        s_PreviousGamepadButtons;
    static std::array<JoystickDevice, 8> s_Joysticks;
    static int s_JoystickCount;
    static std::vector<std::shared_ptr<ICustomInputDevice>> s_CustomDevices;

    static glm::vec2 s_MousePosition;
    static glm::vec2 s_PreviousMousePosition;
    static glm::vec2 s_MouseScroll;
    static glm::vec2 s_PendingMouseScroll;

    static float ApplyDeadzone(float value, float deadzone = 0.1f);
    static bool IsValidJoystickSlot(int id);
    static SDL_Gamepad *GetGamepad(int id);
    static JoystickDevice *GetJoystick(int id);
    static void RefreshGamepads();
    static void RefreshJoysticks();
    static void UpdateJoysticks();

  public:
    // Internal hooks used by the window backend.
    static void OnMouseScroll(double xOffset, double yOffset);
    static void OnGamepadChanged();
    static void OnJoystickChanged();
};

} // namespace axiom

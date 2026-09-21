#include "axiom/input/Input.h"

#include <SDL3/SDL.h>

#include <algorithm>
#include <cmath>

namespace axiom {

    SDL_Window *Input::s_Window = nullptr;

    std::array<bool, SDL_SCANCODE_COUNT> Input::s_CurrentKeys{};
    std::array<bool, SDL_SCANCODE_COUNT> Input::s_PreviousKeys{};

    std::array<bool, 9> Input::s_CurrentMouse{};
    std::array<bool, 9> Input::s_PreviousMouse{};

    std::array<SDL_Gamepad *, 4> Input::s_Gamepads{};
    std::array<std::array<bool, SDL_GAMEPAD_BUTTON_COUNT>, 4>
        Input::s_CurrentGamepadButtons{};
    std::array<std::array<bool, SDL_GAMEPAD_BUTTON_COUNT>, 4>
        Input::s_PreviousGamepadButtons{};
    std::array<Input::JoystickDevice, 8> Input::s_Joysticks{};
    int Input::s_JoystickCount = 0;
    std::vector<std::shared_ptr<ICustomInputDevice>> Input::s_CustomDevices{};

    glm::vec2 Input::s_MousePosition{};
    glm::vec2 Input::s_PreviousMousePosition{};
    glm::vec2 Input::s_MouseScroll{};
    glm::vec2 Input::s_PendingMouseScroll{};

    void Input::Init(void *window) {
        s_Window = static_cast<SDL_Window *>(window);
        RefreshGamepads();
        RefreshJoysticks();
    }

    void Input::Shutdown() {
        for (SDL_Gamepad *&gamepad : s_Gamepads) {
            if (gamepad) {
                SDL_CloseGamepad(gamepad);
                gamepad = nullptr;
            }
        }

        for (JoystickDevice &joystick : s_Joysticks) {
            if (joystick.Handle) {
                SDL_CloseJoystick(joystick.Handle);
                joystick = {};
            }
        }

        s_JoystickCount = 0;
        s_CustomDevices.clear();
        s_Window = nullptr;
    }

    void Input::Update() {
        s_PreviousKeys = s_CurrentKeys;
        s_PreviousMouse = s_CurrentMouse;
        s_PreviousGamepadButtons = s_CurrentGamepadButtons;
        s_PreviousMousePosition = s_MousePosition;
        s_MouseScroll = s_PendingMouseScroll;
        s_PendingMouseScroll = glm::vec2(0.0f);

        int keyCount = 0;
        const bool *keyboardState = SDL_GetKeyboardState(&keyCount);
        const int copyCount =
            std::min(keyCount, static_cast<int>(SDL_SCANCODE_COUNT));
        for (int key = 0; key < copyCount; ++key)
            s_CurrentKeys[key] = keyboardState[key];
        for (int key = copyCount; key < SDL_SCANCODE_COUNT; ++key)
            s_CurrentKeys[key] = false;

        float mouseX = 0.0f;
        float mouseY = 0.0f;
        const SDL_MouseButtonFlags mouseState =
            SDL_GetMouseState(&mouseX, &mouseY);
        s_MousePosition = glm::vec2(mouseX, mouseY);

        for (size_t button = 0; button < s_CurrentMouse.size(); ++button)
            s_CurrentMouse[button] =
                (mouseState & SDL_BUTTON_MASK(static_cast<int>(button))) != 0;

        SDL_UpdateGamepads();
        SDL_UpdateJoysticks();
        UpdateJoysticks();

        for (const std::shared_ptr<ICustomInputDevice> &device :
             s_CustomDevices) {
            if (device)
                device->Update();
        }

        for (size_t device = 0; device < s_Gamepads.size(); ++device) {
            SDL_Gamepad *gamepad = s_Gamepads[device];
            for (int button = 0; button < SDL_GAMEPAD_BUTTON_COUNT; ++button) {
                s_CurrentGamepadButtons[device][button] =
                    gamepad &&
                    SDL_GetGamepadButton(
                        gamepad, static_cast<SDL_GamepadButton>(button));
            }
        }
    }

    float Input::ApplyDeadzone(float value, float deadzone) {
        return std::abs(value) < deadzone ? 0.0f : value;
    }

    bool Input::IsValidJoystickSlot(int id) {
        return id >= 0 && static_cast<size_t>(id) < s_Joysticks.size();
    }

    bool Input::IsKeyPressed(KeyCode key) {
        return key >= 0 && key < SDL_SCANCODE_COUNT && s_CurrentKeys[key];
    }

    bool Input::IsKeyJustPressed(KeyCode key) {
        return key >= 0 && key < SDL_SCANCODE_COUNT && s_CurrentKeys[key] &&
               !s_PreviousKeys[key];
    }

    bool Input::IsKeyReleased(KeyCode key) {
        return key >= 0 && key < SDL_SCANCODE_COUNT && !s_CurrentKeys[key] &&
               s_PreviousKeys[key];
    }

    bool Input::IsMousePressed(MouseCode button) {
        return button >= 0 &&
               static_cast<size_t>(button) < s_CurrentMouse.size() &&
               s_CurrentMouse[button];
    }

    bool Input::IsMouseJustPressed(MouseCode button) {
        return button >= 0 &&
               static_cast<size_t>(button) < s_CurrentMouse.size() &&
               s_CurrentMouse[button] && !s_PreviousMouse[button];
    }

    bool Input::IsMouseReleased(MouseCode button) {
        return button >= 0 &&
               static_cast<size_t>(button) < s_CurrentMouse.size() &&
               !s_CurrentMouse[button] && s_PreviousMouse[button];
    }

    glm::vec2 Input::GetMousePosition() { return s_MousePosition; }

    glm::vec2 Input::GetMouseDelta() {
        return s_MousePosition - s_PreviousMousePosition;
    }

    glm::vec2 Input::GetMouseScroll() { return s_MouseScroll; }

    glm::vec2 Input::GetMouseScrollDelta() { return s_MouseScroll; }

    void Input::OnMouseScroll(double xOffset, double yOffset) {
        s_PendingMouseScroll +=
            glm::vec2(static_cast<float>(xOffset), static_cast<float>(yOffset));
    }

    void Input::RefreshGamepads() {
        for (SDL_Gamepad *&gamepad : s_Gamepads) {
            if (gamepad) {
                SDL_CloseGamepad(gamepad);
                gamepad = nullptr;
            }
        }

        int count = 0;
        SDL_JoystickID *ids = SDL_GetGamepads(&count);
        if (!ids)
            return;

        const int openCount =
            std::min<int>(count, static_cast<int>(s_Gamepads.size()));
        for (int i = 0; i < openCount; ++i)
            s_Gamepads[i] = SDL_OpenGamepad(ids[i]);

        SDL_free(ids);
    }

    void Input::OnGamepadChanged() { RefreshGamepads(); }

    void Input::RefreshJoysticks() {
        for (JoystickDevice &joystick : s_Joysticks) {
            if (joystick.Handle) {
                SDL_CloseJoystick(joystick.Handle);
                joystick = {};
            }
        }

        s_JoystickCount = 0;

        int count = 0;
        SDL_JoystickID *ids = SDL_GetJoysticks(&count);
        if (!ids)
            return;

        const int openCount =
            std::min<int>(count, static_cast<int>(s_Joysticks.size()));
        for (int i = 0; i < openCount; ++i) {
            SDL_Joystick *joystick = SDL_OpenJoystick(ids[i]);
            if (!joystick)
                continue;

            JoystickDevice &device = s_Joysticks[s_JoystickCount++];
            device.Handle = joystick;
            device.InstanceID = SDL_GetJoystickID(joystick);

            const char *name = SDL_GetJoystickName(joystick);
            device.Name = name ? name : "";

            const int axisCount = std::max(0, SDL_GetNumJoystickAxes(joystick));
            const int buttonCount =
                std::max(0, SDL_GetNumJoystickButtons(joystick));
            const int hatCount = std::max(0, SDL_GetNumJoystickHats(joystick));

            device.Axes.assign(axisCount, 0.0f);
            device.RawAxes.assign(axisCount, 0);
            device.CurrentButtons.assign(buttonCount, false);
            device.PreviousButtons.assign(buttonCount, false);
            device.Hats.assign(hatCount, Joystick::HatCentered);
            device.PreviousHats.assign(hatCount, Joystick::HatCentered);
        }

        SDL_free(ids);
        UpdateJoysticks();
    }

    void Input::OnJoystickChanged() { RefreshJoysticks(); }

    Input::JoystickDevice *Input::GetJoystick(int id) {
        if (!IsValidJoystickSlot(id))
            return nullptr;

        if (id >= s_JoystickCount)
            RefreshJoysticks();

        if (id >= s_JoystickCount || !s_Joysticks[id].Handle)
            return nullptr;

        return &s_Joysticks[id];
    }

    void Input::UpdateJoysticks() {
        for (int deviceIndex = 0; deviceIndex < s_JoystickCount;
             ++deviceIndex) {
            JoystickDevice &device = s_Joysticks[deviceIndex];
            if (!device.Handle || !SDL_JoystickConnected(device.Handle))
                continue;

            device.PreviousButtons = device.CurrentButtons;
            device.PreviousHats = device.Hats;

            for (size_t axis = 0; axis < device.RawAxes.size(); ++axis) {
                const int16_t raw =
                    SDL_GetJoystickAxis(device.Handle, static_cast<int>(axis));
                device.RawAxes[axis] = raw;

                const float normalizer =
                    raw < 0 ? static_cast<float>(-SDL_JOYSTICK_AXIS_MIN)
                            : static_cast<float>(SDL_JOYSTICK_AXIS_MAX);
                device.Axes[axis] = glm::clamp(
                    static_cast<float>(raw) / normalizer, -1.0f, 1.0f);
            }

            for (size_t button = 0; button < device.CurrentButtons.size();
                 ++button)
                device.CurrentButtons[button] = SDL_GetJoystickButton(
                    device.Handle, static_cast<int>(button));

            for (size_t hat = 0; hat < device.Hats.size(); ++hat)
                device.Hats[hat] =
                    SDL_GetJoystickHat(device.Handle, static_cast<int>(hat));
        }
    }

    SDL_Gamepad *Input::GetGamepad(int id) {
        if (id < 0 || static_cast<size_t>(id) >= s_Gamepads.size())
            return nullptr;

        if (!s_Gamepads[id])
            RefreshGamepads();

        return s_Gamepads[id];
    }

    bool Input::IsGamepadConnected(int id) { return GetGamepad(id) != nullptr; }

    std::string Input::GetGamepadName(int id) {
        SDL_Gamepad *gamepad = GetGamepad(id);
        if (!gamepad)
            return {};

        const char *name = SDL_GetGamepadName(gamepad);
        return name ? name : "";
    }

    bool Input::IsGamepadPressed(GamepadButton button, int id) {
        if (id < 0 ||
            static_cast<size_t>(id) >= s_CurrentGamepadButtons.size() ||
            button < 0 || button >= SDL_GAMEPAD_BUTTON_COUNT)
            return false;

        return s_CurrentGamepadButtons[id][button];
    }

    bool Input::IsGamepadJustPressed(GamepadButton button, int id) {
        if (id < 0 ||
            static_cast<size_t>(id) >= s_CurrentGamepadButtons.size() ||
            button < 0 || button >= SDL_GAMEPAD_BUTTON_COUNT)
            return false;

        return s_CurrentGamepadButtons[id][button] &&
               !s_PreviousGamepadButtons[id][button];
    }

    bool Input::IsGamepadReleased(GamepadButton button, int id) {
        if (id < 0 ||
            static_cast<size_t>(id) >= s_CurrentGamepadButtons.size() ||
            button < 0 || button >= SDL_GAMEPAD_BUTTON_COUNT)
            return false;

        return !s_CurrentGamepadButtons[id][button] &&
               s_PreviousGamepadButtons[id][button];
    }

    float Input::GetGamepadAxis(GamepadAxis axis, int id) {
        SDL_Gamepad *gamepad = GetGamepad(id);
        if (!gamepad)
            return 0.0f;

        const auto sdlAxis = static_cast<SDL_GamepadAxis>(axis);
        const float value =
            static_cast<float>(SDL_GetGamepadAxis(gamepad, sdlAxis));
        const float normalizer =
            sdlAxis == SDL_GAMEPAD_AXIS_LEFT_TRIGGER ||
                    sdlAxis == SDL_GAMEPAD_AXIS_RIGHT_TRIGGER
                ? static_cast<float>(SDL_JOYSTICK_AXIS_MAX)
                : 32768.0f;

        return ApplyDeadzone(glm::clamp(value / normalizer, -1.0f, 1.0f));
    }

    glm::vec2 Input::GetGamepadLeftStick(int id) {
        return glm::vec2(GetGamepadAxis(SDL_GAMEPAD_AXIS_LEFTX, id),
                         GetGamepadAxis(SDL_GAMEPAD_AXIS_LEFTY, id));
    }

    glm::vec2 Input::GetGamepadRightStick(int id) {
        return glm::vec2(GetGamepadAxis(SDL_GAMEPAD_AXIS_RIGHTX, id),
                         GetGamepadAxis(SDL_GAMEPAD_AXIS_RIGHTY, id));
    }

    float Input::GetGamepadLeftTrigger(int id) {
        return GetGamepadAxis(SDL_GAMEPAD_AXIS_LEFT_TRIGGER, id);
    }

    float Input::GetGamepadRightTrigger(int id) {
        return GetGamepadAxis(SDL_GAMEPAD_AXIS_RIGHT_TRIGGER, id);
    }

    bool Input::RumbleGamepad(uint16_t lowFrequency, uint16_t highFrequency,
                              uint32_t durationMs, int id) {
        SDL_Gamepad *gamepad = GetGamepad(id);
        return gamepad && SDL_RumbleGamepad(gamepad, lowFrequency,
                                            highFrequency, durationMs);
    }

    bool Input::RumbleGamepadTriggers(uint16_t leftTrigger,
                                      uint16_t rightTrigger,
                                      uint32_t durationMs, int id) {
        SDL_Gamepad *gamepad = GetGamepad(id);
        return gamepad && SDL_RumbleGamepadTriggers(gamepad, leftTrigger,
                                                    rightTrigger, durationMs);
    }

    bool Input::SetGamepadLED(uint8_t red, uint8_t green, uint8_t blue,
                              int id) {
        SDL_Gamepad *gamepad = GetGamepad(id);
        return gamepad && SDL_SetGamepadLED(gamepad, red, green, blue);
    }

    bool Input::SetGamepadSensorEnabled(GamepadSensor sensor, bool enabled,
                                        int id) {
        SDL_Gamepad *gamepad = GetGamepad(id);
        return gamepad &&
               SDL_SetGamepadSensorEnabled(
                   gamepad, static_cast<SDL_SensorType>(sensor), enabled);
    }

    bool Input::GetGamepadSensorData(GamepadSensor sensor, float *data,
                                     int valueCount, int id) {
        SDL_Gamepad *gamepad = GetGamepad(id);
        return gamepad && data && valueCount > 0 &&
               SDL_GetGamepadSensorData(gamepad,
                                        static_cast<SDL_SensorType>(sensor),
                                        data, valueCount);
    }

    int Input::GetGamepadTouchpadCount(int id) {
        SDL_Gamepad *gamepad = GetGamepad(id);
        return gamepad ? SDL_GetNumGamepadTouchpads(gamepad) : 0;
    }

    int Input::GetGamepadTouchpadFingerCount(int touchpad, int id) {
        SDL_Gamepad *gamepad = GetGamepad(id);
        return gamepad ? SDL_GetNumGamepadTouchpadFingers(gamepad, touchpad)
                       : 0;
    }

    bool Input::GetGamepadTouchpadFinger(int touchpad, int finger, bool &down,
                                         float &x, float &y, float &pressure,
                                         int id) {
        SDL_Gamepad *gamepad = GetGamepad(id);
        if (!gamepad)
            return false;

        return SDL_GetGamepadTouchpadFinger(gamepad, touchpad, finger, &down,
                                            &x, &y, &pressure);
    }

    bool Input::IsJoystickConnected(int id) {
        return GetJoystick(id) != nullptr;
    }

    std::string Input::GetJoystickName(int id) {
        JoystickDevice *joystick = GetJoystick(id);
        return joystick ? joystick->Name : "";
    }

    int Input::GetJoystickCount() { return s_JoystickCount; }

    int Input::GetJoystickAxisCount(int id) {
        JoystickDevice *joystick = GetJoystick(id);
        return joystick ? static_cast<int>(joystick->Axes.size()) : 0;
    }

    int Input::GetJoystickButtonCount(int id) {
        JoystickDevice *joystick = GetJoystick(id);
        return joystick ? static_cast<int>(joystick->CurrentButtons.size()) : 0;
    }

    int Input::GetJoystickHatCount(int id) {
        JoystickDevice *joystick = GetJoystick(id);
        return joystick ? static_cast<int>(joystick->Hats.size()) : 0;
    }

    float Input::GetJoystickAxis(JoystickAxis axis, int id, float deadzone) {
        JoystickDevice *joystick = GetJoystick(id);
        if (!joystick || axis < 0 ||
            static_cast<size_t>(axis) >= joystick->Axes.size())
            return 0.0f;

        return ApplyDeadzone(joystick->Axes[axis], deadzone);
    }

    int16_t Input::GetJoystickAxisRaw(JoystickAxis axis, int id) {
        JoystickDevice *joystick = GetJoystick(id);
        if (!joystick || axis < 0 ||
            static_cast<size_t>(axis) >= joystick->RawAxes.size())
            return 0;

        return joystick->RawAxes[axis];
    }

    bool Input::IsJoystickButtonPressed(JoystickButton button, int id) {
        JoystickDevice *joystick = GetJoystick(id);
        if (!joystick || button < 0 ||
            static_cast<size_t>(button) >= joystick->CurrentButtons.size())
            return false;

        return joystick->CurrentButtons[button];
    }

    bool Input::IsJoystickButtonJustPressed(JoystickButton button, int id) {
        JoystickDevice *joystick = GetJoystick(id);
        if (!joystick || button < 0 ||
            static_cast<size_t>(button) >= joystick->CurrentButtons.size())
            return false;

        return joystick->CurrentButtons[button] &&
               !joystick->PreviousButtons[button];
    }

    bool Input::IsJoystickButtonReleased(JoystickButton button, int id) {
        JoystickDevice *joystick = GetJoystick(id);
        if (!joystick || button < 0 ||
            static_cast<size_t>(button) >= joystick->CurrentButtons.size())
            return false;

        return !joystick->CurrentButtons[button] &&
               joystick->PreviousButtons[button];
    }

    JoystickHat Input::GetJoystickHat(int hat, int id) {
        JoystickDevice *joystick = GetJoystick(id);
        if (!joystick || hat < 0 ||
            static_cast<size_t>(hat) >= joystick->Hats.size())
            return Joystick::HatCentered;

        return joystick->Hats[hat];
    }

    bool Input::IsJoystickHatPressed(JoystickHat mask, int hat, int id) {
        return (GetJoystickHat(hat, id) & mask) != 0;
    }

    bool Input::IsJoystickHatJustPressed(JoystickHat mask, int hat, int id) {
        JoystickDevice *joystick = GetJoystick(id);
        if (!joystick || hat < 0 ||
            static_cast<size_t>(hat) >= joystick->Hats.size())
            return false;

        return (joystick->Hats[hat] & mask) != 0 &&
               (joystick->PreviousHats[hat] & mask) == 0;
    }

    bool Input::IsJoystickHatReleased(JoystickHat mask, int hat, int id) {
        JoystickDevice *joystick = GetJoystick(id);
        if (!joystick || hat < 0 ||
            static_cast<size_t>(hat) >= joystick->Hats.size())
            return false;

        return (joystick->Hats[hat] & mask) == 0 &&
               (joystick->PreviousHats[hat] & mask) != 0;
    }

    bool Input::RumbleJoystick(uint16_t lowFrequency, uint16_t highFrequency,
                               uint32_t durationMs, int id) {
        JoystickDevice *joystick = GetJoystick(id);
        return joystick && SDL_RumbleJoystick(joystick->Handle, lowFrequency,
                                              highFrequency, durationMs);
    }

    bool Input::RumbleJoystickTriggers(uint16_t leftTrigger,
                                       uint16_t rightTrigger,
                                       uint32_t durationMs, int id) {
        JoystickDevice *joystick = GetJoystick(id);
        return joystick &&
               SDL_RumbleJoystickTriggers(joystick->Handle, leftTrigger,
                                          rightTrigger, durationMs);
    }

    bool Input::SetJoystickLED(uint8_t red, uint8_t green, uint8_t blue,
                               int id) {
        JoystickDevice *joystick = GetJoystick(id);
        return joystick &&
               SDL_SetJoystickLED(joystick->Handle, red, green, blue);
    }

    int
    Input::RegisterCustomDevice(std::shared_ptr<ICustomInputDevice> device) {
        if (!device)
            return -1;

        for (size_t i = 0; i < s_CustomDevices.size(); ++i) {
            if (!s_CustomDevices[i]) {
                s_CustomDevices[i] = std::move(device);
                return static_cast<int>(i);
            }
        }

        s_CustomDevices.push_back(std::move(device));
        return static_cast<int>(s_CustomDevices.size() - 1);
    }

    void Input::UnregisterCustomDevice(int id) {
        if (id < 0 || static_cast<size_t>(id) >= s_CustomDevices.size())
            return;

        s_CustomDevices[id].reset();
    }

    void Input::ClearCustomDevices() { s_CustomDevices.clear(); }

    bool Input::IsCustomDeviceConnected(int id) {
        return id >= 0 && static_cast<size_t>(id) < s_CustomDevices.size() &&
               s_CustomDevices[id] != nullptr;
    }

    std::string Input::GetCustomDeviceName(int id) {
        if (!IsCustomDeviceConnected(id))
            return "";

        return s_CustomDevices[id]->GetName();
    }

    int Input::GetCustomDeviceCount() {
        return static_cast<int>(std::count_if(
            s_CustomDevices.begin(), s_CustomDevices.end(),
            [](const std::shared_ptr<ICustomInputDevice> &device) {
                return device != nullptr;
            }));
    }

    int Input::GetCustomDeviceAxisCount(int id) {
        return IsCustomDeviceConnected(id) ? s_CustomDevices[id]->GetAxisCount()
                                           : 0;
    }

    int Input::GetCustomDeviceButtonCount(int id) {
        return IsCustomDeviceConnected(id)
                   ? s_CustomDevices[id]->GetButtonCount()
                   : 0;
    }

    int Input::GetCustomDeviceHatCount(int id) {
        return IsCustomDeviceConnected(id) ? s_CustomDevices[id]->GetHatCount()
                                           : 0;
    }

    float Input::GetCustomDeviceAxis(int axis, int id, float deadzone) {
        if (!IsCustomDeviceConnected(id) || axis < 0 ||
            axis >= s_CustomDevices[id]->GetAxisCount())
            return 0.0f;

        return ApplyDeadzone(
            glm::clamp(s_CustomDevices[id]->GetAxis(axis), -1.0f, 1.0f),
            deadzone);
    }

    bool Input::IsCustomDeviceButtonPressed(int button, int id) {
        return IsCustomDeviceConnected(id) && button >= 0 &&
               button < s_CustomDevices[id]->GetButtonCount() &&
               s_CustomDevices[id]->IsButtonPressed(button);
    }

    bool Input::IsCustomDeviceButtonJustPressed(int button, int id) {
        return IsCustomDeviceConnected(id) && button >= 0 &&
               button < s_CustomDevices[id]->GetButtonCount() &&
               s_CustomDevices[id]->IsButtonJustPressed(button);
    }

    bool Input::IsCustomDeviceButtonReleased(int button, int id) {
        return IsCustomDeviceConnected(id) && button >= 0 &&
               button < s_CustomDevices[id]->GetButtonCount() &&
               s_CustomDevices[id]->IsButtonReleased(button);
    }

    JoystickHat Input::GetCustomDeviceHat(int hat, int id) {
        if (!IsCustomDeviceConnected(id) || hat < 0 ||
            hat >= s_CustomDevices[id]->GetHatCount())
            return Joystick::HatCentered;

        return s_CustomDevices[id]->GetHat(hat);
    }

    bool Input::IsCustomDeviceHatPressed(JoystickHat mask, int hat, int id) {
        return (GetCustomDeviceHat(hat, id) & mask) != 0;
    }

    bool Input::IsCustomDeviceHatJustPressed(JoystickHat mask, int hat,
                                             int id) {
        return IsCustomDeviceConnected(id) && hat >= 0 &&
               hat < s_CustomDevices[id]->GetHatCount() &&
               s_CustomDevices[id]->IsHatJustPressed(mask, hat);
    }

    bool Input::IsCustomDeviceHatReleased(JoystickHat mask, int hat, int id) {
        return IsCustomDeviceConnected(id) && hat >= 0 &&
               hat < s_CustomDevices[id]->GetHatCount() &&
               s_CustomDevices[id]->IsHatReleased(mask, hat);
    }

} // namespace axiom

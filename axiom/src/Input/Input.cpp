/**
 * @file axiom/src/Input/Input.cpp
 * @brief Implements the input module.
 */

#include "axiom/input/Input.h"

#include <SDL3/SDL.h>

#include <algorithm>
#include <cmath>

namespace axiom {

    SDL_Window *Input::sWindow = nullptr;

    std::array<bool, SDL_SCANCODE_COUNT> Input::sCurrentKeys{};
    std::array<bool, SDL_SCANCODE_COUNT> Input::sPreviousKeys{};

    std::array<bool, 9> Input::sCurrentMouse{};
    std::array<bool, 9> Input::sPreviousMouse{};

    std::array<SDL_Gamepad *, 4> Input::sGamepads{};
    std::array<std::array<bool, SDL_GAMEPAD_BUTTON_COUNT>, 4>
        Input::sCurrentGamepadButtons{};
    std::array<std::array<bool, SDL_GAMEPAD_BUTTON_COUNT>, 4>
        Input::sPreviousGamepadButtons{};
    std::array<Input::JoystickDevice, 8> Input::sJoysticks{};
    int Input::sJoystickCount = 0;
    std::vector<std::shared_ptr<ICustomInputDevice>> Input::sCustomDevices{};

    glm::vec2 Input::sMousePosition{};
    glm::vec2 Input::sPreviousMousePosition{};
    glm::vec2 Input::sMouseScroll{};
    glm::vec2 Input::sPendingMouseScroll{};

    void Input::init(void *window) {
        sWindow = static_cast<SDL_Window *>(window);
        refreshGamepads();
        refreshJoysticks();
    }

    void Input::shutdown() {
        for (SDL_Gamepad *&gamepad : sGamepads) {
            if (gamepad) {
                SDL_CloseGamepad(gamepad);
                gamepad = nullptr;
            }
        }

        for (JoystickDevice &joystick : sJoysticks) {
            if (joystick.Handle) {
                SDL_CloseJoystick(joystick.Handle);
                joystick = {};
            }
        }

        sJoystickCount = 0;
        sCustomDevices.clear();
        sWindow = nullptr;
    }

    void Input::update() {
        sPreviousKeys = sCurrentKeys;
        sPreviousMouse = sCurrentMouse;
        sPreviousGamepadButtons = sCurrentGamepadButtons;
        sPreviousMousePosition = sMousePosition;
        sMouseScroll = sPendingMouseScroll;
        sPendingMouseScroll = glm::vec2(0.0f);

        int keyCount = 0;
        const bool *keyboardState = SDL_GetKeyboardState(&keyCount);
        const int copyCount =
            std::min(keyCount, static_cast<int>(SDL_SCANCODE_COUNT));
        for (int key = 0; key < copyCount; ++key)
            sCurrentKeys[key] = keyboardState[key];
        for (int key = copyCount; key < SDL_SCANCODE_COUNT; ++key)
            sCurrentKeys[key] = false;

        float mouseX = 0.0f;
        float mouseY = 0.0f;
        const SDL_MouseButtonFlags mouseState =
            SDL_GetMouseState(&mouseX, &mouseY);
        sMousePosition = glm::vec2(mouseX, mouseY);

        for (size_t button = 0; button < sCurrentMouse.size(); ++button)
            sCurrentMouse[button] =
                (mouseState & SDL_BUTTON_MASK(static_cast<int>(button))) != 0;

        SDL_UpdateGamepads();
        SDL_UpdateJoysticks();
        updateJoysticks();

        for (const std::shared_ptr<ICustomInputDevice> &device :
             sCustomDevices) {
            if (device)
                device->update();
        }

        for (size_t device = 0; device < sGamepads.size(); ++device) {
            SDL_Gamepad *gamepad = sGamepads[device];
            for (int button = 0; button < SDL_GAMEPAD_BUTTON_COUNT; ++button) {
                sCurrentGamepadButtons[device][button] =
                    gamepad &&
                    SDL_GetGamepadButton(
                        gamepad, static_cast<SDL_GamepadButton>(button));
            }
        }
    }

    float Input::applyDeadzone(float value, float deadzone) {
        return std::abs(value) < deadzone ? 0.0f : value;
    }

    bool Input::isValidJoystickSlot(int id) {
        return id >= 0 && static_cast<size_t>(id) < sJoysticks.size();
    }

    bool Input::isKeyPressed(KeyCode key) {
        return key >= 0 && key < SDL_SCANCODE_COUNT && sCurrentKeys[key];
    }

    bool Input::isKeyJustPressed(KeyCode key) {
        return key >= 0 && key < SDL_SCANCODE_COUNT && sCurrentKeys[key] &&
               !sPreviousKeys[key];
    }

    bool Input::isKeyReleased(KeyCode key) {
        return key >= 0 && key < SDL_SCANCODE_COUNT && !sCurrentKeys[key] &&
               sPreviousKeys[key];
    }

    bool Input::isMousePressed(MouseCode button) {
        return button >= 0 &&
               static_cast<size_t>(button) < sCurrentMouse.size() &&
               sCurrentMouse[button];
    }

    bool Input::isMouseJustPressed(MouseCode button) {
        return button >= 0 &&
               static_cast<size_t>(button) < sCurrentMouse.size() &&
               sCurrentMouse[button] && !sPreviousMouse[button];
    }

    bool Input::isMouseReleased(MouseCode button) {
        return button >= 0 &&
               static_cast<size_t>(button) < sCurrentMouse.size() &&
               !sCurrentMouse[button] && sPreviousMouse[button];
    }

    glm::vec2 Input::getMousePosition() { return sMousePosition; }

    glm::vec2 Input::getMouseDelta() {
        return sMousePosition - sPreviousMousePosition;
    }

    glm::vec2 Input::getMouseScroll() { return sMouseScroll; }

    glm::vec2 Input::getMouseScrollDelta() { return sMouseScroll; }

    void Input::onMouseScroll(double xOffset, double yOffset) {
        sPendingMouseScroll +=
            glm::vec2(static_cast<float>(xOffset), static_cast<float>(yOffset));
    }

    void Input::refreshGamepads() {
        for (SDL_Gamepad *&gamepad : sGamepads) {
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
            std::min<int>(count, static_cast<int>(sGamepads.size()));
        for (int i = 0; i < openCount; ++i)
            sGamepads[i] = SDL_OpenGamepad(ids[i]);

        SDL_free(ids);
    }

    void Input::onGamepadChanged() { refreshGamepads(); }

    void Input::refreshJoysticks() {
        for (JoystickDevice &joystick : sJoysticks) {
            if (joystick.Handle) {
                SDL_CloseJoystick(joystick.Handle);
                joystick = {};
            }
        }

        sJoystickCount = 0;

        int count = 0;
        SDL_JoystickID *ids = SDL_GetJoysticks(&count);
        if (!ids)
            return;

        const int openCount =
            std::min<int>(count, static_cast<int>(sJoysticks.size()));
        for (int i = 0; i < openCount; ++i) {
            SDL_Joystick *joystick = SDL_OpenJoystick(ids[i]);
            if (!joystick)
                continue;

            JoystickDevice &device = sJoysticks[sJoystickCount++];
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
            device.Hats.assign(hatCount, joystick::hatCentered);
            device.PreviousHats.assign(hatCount, joystick::hatCentered);
        }

        SDL_free(ids);
        updateJoysticks();
    }

    void Input::onJoystickChanged() { refreshJoysticks(); }

    Input::JoystickDevice *Input::getJoystick(int id) {
        if (!isValidJoystickSlot(id))
            return nullptr;

        if (id >= sJoystickCount)
            refreshJoysticks();

        if (id >= sJoystickCount || !sJoysticks[id].Handle)
            return nullptr;

        return &sJoysticks[id];
    }

    void Input::updateJoysticks() {
        for (int deviceIndex = 0; deviceIndex < sJoystickCount;
             ++deviceIndex) {
            JoystickDevice &device = sJoysticks[deviceIndex];
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

    SDL_Gamepad *Input::getGamepad(int id) {
        if (id < 0 || static_cast<size_t>(id) >= sGamepads.size())
            return nullptr;

        if (!sGamepads[id])
            refreshGamepads();

        return sGamepads[id];
    }

    bool Input::isGamepadConnected(int id) { return getGamepad(id) != nullptr; }

    std::string Input::getGamepadName(int id) {
        SDL_Gamepad *gamepad = getGamepad(id);
        if (!gamepad)
            return {};

        const char *name = SDL_GetGamepadName(gamepad);
        return name ? name : "";
    }

    bool Input::isGamepadPressed(GamepadButton button, int id) {
        if (id < 0 ||
            static_cast<size_t>(id) >= sCurrentGamepadButtons.size() ||
            button < 0 || button >= SDL_GAMEPAD_BUTTON_COUNT)
            return false;

        return sCurrentGamepadButtons[id][button];
    }

    bool Input::isGamepadJustPressed(GamepadButton button, int id) {
        if (id < 0 ||
            static_cast<size_t>(id) >= sCurrentGamepadButtons.size() ||
            button < 0 || button >= SDL_GAMEPAD_BUTTON_COUNT)
            return false;

        return sCurrentGamepadButtons[id][button] &&
               !sPreviousGamepadButtons[id][button];
    }

    bool Input::isGamepadReleased(GamepadButton button, int id) {
        if (id < 0 ||
            static_cast<size_t>(id) >= sCurrentGamepadButtons.size() ||
            button < 0 || button >= SDL_GAMEPAD_BUTTON_COUNT)
            return false;

        return !sCurrentGamepadButtons[id][button] &&
               sPreviousGamepadButtons[id][button];
    }

    float Input::getGamepadAxis(GamepadAxis axis, int id) {
        SDL_Gamepad *gamepad = getGamepad(id);
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

        return applyDeadzone(glm::clamp(value / normalizer, -1.0f, 1.0f));
    }

    glm::vec2 Input::getGamepadLeftStick(int id) {
        return glm::vec2(getGamepadAxis(SDL_GAMEPAD_AXIS_LEFTX, id),
                         getGamepadAxis(SDL_GAMEPAD_AXIS_LEFTY, id));
    }

    glm::vec2 Input::getGamepadRightStick(int id) {
        return glm::vec2(getGamepadAxis(SDL_GAMEPAD_AXIS_RIGHTX, id),
                         getGamepadAxis(SDL_GAMEPAD_AXIS_RIGHTY, id));
    }

    float Input::getGamepadLeftTrigger(int id) {
        return getGamepadAxis(SDL_GAMEPAD_AXIS_LEFT_TRIGGER, id);
    }

    float Input::getGamepadRightTrigger(int id) {
        return getGamepadAxis(SDL_GAMEPAD_AXIS_RIGHT_TRIGGER, id);
    }

    bool Input::rumbleGamepad(uint16_t lowFrequency, uint16_t highFrequency,
                              uint32_t durationMs, int id) {
        SDL_Gamepad *gamepad = getGamepad(id);
        return gamepad && SDL_RumbleGamepad(gamepad, lowFrequency,
                                            highFrequency, durationMs);
    }

    bool Input::rumbleGamepadTriggers(uint16_t leftTrigger,
                                      uint16_t rightTrigger,
                                      uint32_t durationMs, int id) {
        SDL_Gamepad *gamepad = getGamepad(id);
        return gamepad && SDL_RumbleGamepadTriggers(gamepad, leftTrigger,
                                                    rightTrigger, durationMs);
    }

    bool Input::setGamepadLed(uint8_t red, uint8_t green, uint8_t blue,
                              int id) {
        SDL_Gamepad *gamepad = getGamepad(id);
        return gamepad && SDL_SetGamepadLED(gamepad, red, green, blue);
    }

    bool Input::setGamepadSensorEnabled(GamepadSensor sensor, bool enabled,
                                        int id) {
        SDL_Gamepad *gamepad = getGamepad(id);
        return gamepad &&
               SDL_SetGamepadSensorEnabled(
                   gamepad, static_cast<SDL_SensorType>(sensor), enabled);
    }

    bool Input::getGamepadSensorData(GamepadSensor sensor, float *data,
                                     int valueCount, int id) {
        SDL_Gamepad *gamepad = getGamepad(id);
        return gamepad && data && valueCount > 0 &&
               SDL_GetGamepadSensorData(gamepad,
                                        static_cast<SDL_SensorType>(sensor),
                                        data, valueCount);
    }

    int Input::getGamepadTouchpadCount(int id) {
        SDL_Gamepad *gamepad = getGamepad(id);
        return gamepad ? SDL_GetNumGamepadTouchpads(gamepad) : 0;
    }

    int Input::getGamepadTouchpadFingerCount(int touchpad, int id) {
        SDL_Gamepad *gamepad = getGamepad(id);
        return gamepad ? SDL_GetNumGamepadTouchpadFingers(gamepad, touchpad)
                       : 0;
    }

    bool Input::getGamepadTouchpadFinger(int touchpad, int finger, bool &down,
                                         float &x, float &y, float &pressure,
                                         int id) {
        SDL_Gamepad *gamepad = getGamepad(id);
        if (!gamepad)
            return false;

        return SDL_GetGamepadTouchpadFinger(gamepad, touchpad, finger, &down,
                                            &x, &y, &pressure);
    }

    bool Input::isJoystickConnected(int id) {
        return getJoystick(id) != nullptr;
    }

    std::string Input::getJoystickName(int id) {
        JoystickDevice *joystick = getJoystick(id);
        return joystick ? joystick->Name : "";
    }

    int Input::getJoystickCount() { return sJoystickCount; }

    int Input::getJoystickAxisCount(int id) {
        JoystickDevice *joystick = getJoystick(id);
        return joystick ? static_cast<int>(joystick->Axes.size()) : 0;
    }

    int Input::getJoystickButtonCount(int id) {
        JoystickDevice *joystick = getJoystick(id);
        return joystick ? static_cast<int>(joystick->CurrentButtons.size()) : 0;
    }

    int Input::getJoystickHatCount(int id) {
        JoystickDevice *joystick = getJoystick(id);
        return joystick ? static_cast<int>(joystick->Hats.size()) : 0;
    }

    float Input::getJoystickAxis(JoystickAxis axis, int id, float deadzone) {
        JoystickDevice *joystick = getJoystick(id);
        if (!joystick || axis < 0 ||
            static_cast<size_t>(axis) >= joystick->Axes.size())
            return 0.0f;

        return applyDeadzone(joystick->Axes[axis], deadzone);
    }

    int16_t Input::getJoystickAxisRaw(JoystickAxis axis, int id) {
        JoystickDevice *joystick = getJoystick(id);
        if (!joystick || axis < 0 ||
            static_cast<size_t>(axis) >= joystick->RawAxes.size())
            return 0;

        return joystick->RawAxes[axis];
    }

    bool Input::isJoystickButtonPressed(JoystickButton button, int id) {
        JoystickDevice *joystick = getJoystick(id);
        if (!joystick || button < 0 ||
            static_cast<size_t>(button) >= joystick->CurrentButtons.size())
            return false;

        return joystick->CurrentButtons[button];
    }

    bool Input::isJoystickButtonJustPressed(JoystickButton button, int id) {
        JoystickDevice *joystick = getJoystick(id);
        if (!joystick || button < 0 ||
            static_cast<size_t>(button) >= joystick->CurrentButtons.size())
            return false;

        return joystick->CurrentButtons[button] &&
               !joystick->PreviousButtons[button];
    }

    bool Input::isJoystickButtonReleased(JoystickButton button, int id) {
        JoystickDevice *joystick = getJoystick(id);
        if (!joystick || button < 0 ||
            static_cast<size_t>(button) >= joystick->CurrentButtons.size())
            return false;

        return !joystick->CurrentButtons[button] &&
               joystick->PreviousButtons[button];
    }

    JoystickHat Input::getJoystickHat(int hat, int id) {
        JoystickDevice *joystick = getJoystick(id);
        if (!joystick || hat < 0 ||
            static_cast<size_t>(hat) >= joystick->Hats.size())
            return joystick::hatCentered;

        return joystick->Hats[hat];
    }

    bool Input::isJoystickHatPressed(JoystickHat mask, int hat, int id) {
        return (getJoystickHat(hat, id) & mask) != 0;
    }

    bool Input::isJoystickHatJustPressed(JoystickHat mask, int hat, int id) {
        JoystickDevice *joystick = getJoystick(id);
        if (!joystick || hat < 0 ||
            static_cast<size_t>(hat) >= joystick->Hats.size())
            return false;

        return (joystick->Hats[hat] & mask) != 0 &&
               (joystick->PreviousHats[hat] & mask) == 0;
    }

    bool Input::isJoystickHatReleased(JoystickHat mask, int hat, int id) {
        JoystickDevice *joystick = getJoystick(id);
        if (!joystick || hat < 0 ||
            static_cast<size_t>(hat) >= joystick->Hats.size())
            return false;

        return (joystick->Hats[hat] & mask) == 0 &&
               (joystick->PreviousHats[hat] & mask) != 0;
    }

    bool Input::rumbleJoystick(uint16_t lowFrequency, uint16_t highFrequency,
                               uint32_t durationMs, int id) {
        JoystickDevice *joystick = getJoystick(id);
        return joystick && SDL_RumbleJoystick(joystick->Handle, lowFrequency,
                                              highFrequency, durationMs);
    }

    bool Input::rumbleJoystickTriggers(uint16_t leftTrigger,
                                       uint16_t rightTrigger,
                                       uint32_t durationMs, int id) {
        JoystickDevice *joystick = getJoystick(id);
        return joystick &&
               SDL_RumbleJoystickTriggers(joystick->Handle, leftTrigger,
                                          rightTrigger, durationMs);
    }

    bool Input::setJoystickLed(uint8_t red, uint8_t green, uint8_t blue,
                               int id) {
        JoystickDevice *joystick = getJoystick(id);
        return joystick &&
               SDL_SetJoystickLED(joystick->Handle, red, green, blue);
    }

    int
    Input::registerCustomDevice(std::shared_ptr<ICustomInputDevice> device) {
        if (!device)
            return -1;

        for (size_t i = 0; i < sCustomDevices.size(); ++i) {
            if (!sCustomDevices[i]) {
                sCustomDevices[i] = std::move(device);
                return static_cast<int>(i);
            }
        }

        sCustomDevices.push_back(std::move(device));
        return static_cast<int>(sCustomDevices.size() - 1);
    }

    void Input::unregisterCustomDevice(int id) {
        if (id < 0 || static_cast<size_t>(id) >= sCustomDevices.size())
            return;

        sCustomDevices[id].reset();
    }

    void Input::clearCustomDevices() { sCustomDevices.clear(); }

    bool Input::isCustomDeviceConnected(int id) {
        return id >= 0 && static_cast<size_t>(id) < sCustomDevices.size() &&
               sCustomDevices[id] != nullptr;
    }

    std::string Input::getCustomDeviceName(int id) {
        if (!isCustomDeviceConnected(id))
            return "";

        return sCustomDevices[id]->getName();
    }

    int Input::getCustomDeviceCount() {
        return static_cast<int>(std::count_if(
            sCustomDevices.begin(), sCustomDevices.end(),
            [](const std::shared_ptr<ICustomInputDevice> &device) {
                return device != nullptr;
            }));
    }

    int Input::getCustomDeviceAxisCount(int id) {
        return isCustomDeviceConnected(id) ? sCustomDevices[id]->getAxisCount()
                                           : 0;
    }

    int Input::getCustomDeviceButtonCount(int id) {
        return isCustomDeviceConnected(id)
                   ? sCustomDevices[id]->getButtonCount()
                   : 0;
    }

    int Input::getCustomDeviceHatCount(int id) {
        return isCustomDeviceConnected(id) ? sCustomDevices[id]->getHatCount()
                                           : 0;
    }

    float Input::getCustomDeviceAxis(int axis, int id, float deadzone) {
        if (!isCustomDeviceConnected(id) || axis < 0 ||
            axis >= sCustomDevices[id]->getAxisCount())
            return 0.0f;

        return applyDeadzone(
            glm::clamp(sCustomDevices[id]->getAxis(axis), -1.0f, 1.0f),
            deadzone);
    }

    bool Input::isCustomDeviceButtonPressed(int button, int id) {
        return isCustomDeviceConnected(id) && button >= 0 &&
               button < sCustomDevices[id]->getButtonCount() &&
               sCustomDevices[id]->isButtonPressed(button);
    }

    bool Input::isCustomDeviceButtonJustPressed(int button, int id) {
        return isCustomDeviceConnected(id) && button >= 0 &&
               button < sCustomDevices[id]->getButtonCount() &&
               sCustomDevices[id]->isButtonJustPressed(button);
    }

    bool Input::isCustomDeviceButtonReleased(int button, int id) {
        return isCustomDeviceConnected(id) && button >= 0 &&
               button < sCustomDevices[id]->getButtonCount() &&
               sCustomDevices[id]->isButtonReleased(button);
    }

    JoystickHat Input::getCustomDeviceHat(int hat, int id) {
        if (!isCustomDeviceConnected(id) || hat < 0 ||
            hat >= sCustomDevices[id]->getHatCount())
            return joystick::hatCentered;

        return sCustomDevices[id]->getHat(hat);
    }

    bool Input::isCustomDeviceHatPressed(JoystickHat mask, int hat, int id) {
        return (getCustomDeviceHat(hat, id) & mask) != 0;
    }

    bool Input::isCustomDeviceHatJustPressed(JoystickHat mask, int hat,
                                             int id) {
        return isCustomDeviceConnected(id) && hat >= 0 &&
               hat < sCustomDevices[id]->getHatCount() &&
               sCustomDevices[id]->isHatJustPressed(mask, hat);
    }

    bool Input::isCustomDeviceHatReleased(JoystickHat mask, int hat, int id) {
        return isCustomDeviceConnected(id) && hat >= 0 &&
               hat < sCustomDevices[id]->getHatCount() &&
               sCustomDevices[id]->isHatReleased(mask, hat);
    }

} // namespace axiom

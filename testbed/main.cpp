// main.cpp
#include <axiom/Axiom.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <string>

class Testbed : public axiom::Application {
public:
    Testbed()
        : Application("Axiom Controller Test") {
    }

protected:
    void OnInit() override {
        axiom::VFS::MountPath("game://", AXIOM_GAME_ASSET_PATH);

        m_Render2DSystem = RegisterSystem<axiom::Render2DSystem>(GetWidth(), GetHeigth());

        BuildControllerScene(GetScene());
        BuildOverlayScene(CreateScene());
    }

    void OnUpdate(double dt) override {
        UpdateCamera(dt);
        UpdateControllerView();
        UpdateOverlayScene();
    }

private:
    axiom::Entity CreateQuad(
        axiom::Scene& scene,
        const std::string& name,
        const glm::vec3& position,
        const glm::vec2& size,
        const glm::vec4& color
    ) {
        axiom::Entity entity = scene.CreateEntity(name);
        auto& transform = entity.GetComponent<axiom::TransformComponent>();
        transform.Translation = position;
        transform.Scale = glm::vec3(size, 1.0f);
        entity.AddComponent<axiom::SpriteRendererComponent>(color);
        return entity;
    }

    axiom::Entity CreateCircle(
        axiom::Scene& scene,
        const std::string& name,
        const glm::vec3& position,
        float radius,
        const glm::vec4& color,
        float thickness = 1.0f
    ) {
        axiom::Entity entity = scene.CreateEntity(name);
        auto& transform = entity.GetComponent<axiom::TransformComponent>();
        transform.Translation = position;
        transform.Scale = glm::vec3(radius * 2.0f, radius * 2.0f, 1.0f);
        entity.AddComponent<axiom::CircleRendererComponent>(color, thickness);
        return entity;
    }

    void SetColor(axiom::Entity entity, const glm::vec4& color) {
        if (entity.HasComponent<axiom::SpriteRendererComponent>())
            entity.GetComponent<axiom::SpriteRendererComponent>().Color = color;
        if (entity.HasComponent<axiom::CircleRendererComponent>())
            entity.GetComponent<axiom::CircleRendererComponent>().Color = color;
    }

    void SetTransform(axiom::Entity entity, const glm::vec3& position, const glm::vec2& size) {
        auto& transform = entity.GetComponent<axiom::TransformComponent>();
        transform.Translation = position;
        transform.Scale = glm::vec3(size, 1.0f);
    }

    void BuildCamera(axiom::Scene& scene, axiom::Entity& cameraEntity, bool primary, float size) {
        cameraEntity = scene.CreateEntity(primary ? "Primary Camera" : "Overlay Camera");
        auto& camera = cameraEntity.AddComponent<axiom::CameraComponent>(primary);
        camera.OrthographicSize = size;
    }

    void BuildControllerScene(axiom::Scene& scene) {
        BuildCamera(scene, m_MainCamera, true, m_CameraSize);

        m_ConnectionIndicator = CreateQuad(scene, "Connection Indicator", {-85.0f, 72.0f, 0.2f}, {18.0f, 8.0f}, m_DisconnectedColor);
        m_ControllerBody = CreateQuad(scene, "Controller Body", {0.0f, 0.0f, 0.0f}, {130.0f, 58.0f}, {0.12f, 0.15f, 0.18f, 1.0f});
        m_Touchpad = CreateQuad(scene, "Touchpad", {0.0f, 18.0f, 0.2f}, {34.0f, 18.0f}, {0.24f, 0.27f, 0.32f, 1.0f});
        m_TouchpadFinger = CreateCircle(scene, "Touchpad Finger", {0.0f, 18.0f, 0.35f}, 2.2f, {0.18f, 0.95f, 0.62f, 0.0f});
        m_LedSwatch = CreateQuad(scene, "LED Swatch", {0.0f, -39.0f, 0.2f}, {42.0f, 5.0f}, {0.2f, 0.3f, 0.8f, 1.0f});

        m_LeftStickBase = CreateCircle(scene, "Left Stick Base", {-38.0f, -7.0f, 0.2f}, 13.0f, {0.08f, 0.09f, 0.11f, 1.0f});
        m_LeftStick = CreateCircle(scene, "Left Stick", {-38.0f, -7.0f, 0.3f}, 7.0f, {0.38f, 0.46f, 0.56f, 1.0f});
        m_RightStickBase = CreateCircle(scene, "Right Stick Base", {38.0f, -7.0f, 0.2f}, 13.0f, {0.08f, 0.09f, 0.11f, 1.0f});
        m_RightStick = CreateCircle(scene, "Right Stick", {38.0f, -7.0f, 0.3f}, 7.0f, {0.38f, 0.46f, 0.56f, 1.0f});

        m_DPad[0] = CreateQuad(scene, "DPad Up", {-66.0f, 15.0f, 0.3f}, {8.0f, 12.0f}, m_IdleColor);
        m_DPad[1] = CreateQuad(scene, "DPad Right", {-56.0f, 5.0f, 0.3f}, {12.0f, 8.0f}, m_IdleColor);
        m_DPad[2] = CreateQuad(scene, "DPad Down", {-66.0f, -5.0f, 0.3f}, {8.0f, 12.0f}, m_IdleColor);
        m_DPad[3] = CreateQuad(scene, "DPad Left", {-76.0f, 5.0f, 0.3f}, {12.0f, 8.0f}, m_IdleColor);

        m_FaceButtons[0] = CreateCircle(scene, "Cross", {66.0f, -8.0f, 0.3f}, 5.0f, m_IdleColor);
        m_FaceButtons[1] = CreateCircle(scene, "Circle", {77.0f, 3.0f, 0.3f}, 5.0f, m_IdleColor);
        m_FaceButtons[2] = CreateCircle(scene, "Square", {55.0f, 3.0f, 0.3f}, 5.0f, m_IdleColor);
        m_FaceButtons[3] = CreateCircle(scene, "Triangle", {66.0f, 14.0f, 0.3f}, 5.0f, m_IdleColor);

        m_ShoulderButtons[0] = CreateQuad(scene, "Left Shoulder", {-45.0f, 38.0f, 0.3f}, {32.0f, 8.0f}, m_IdleColor);
        m_ShoulderButtons[1] = CreateQuad(scene, "Right Shoulder", {45.0f, 38.0f, 0.3f}, {32.0f, 8.0f}, m_IdleColor);
        m_TriggerFill[0] = CreateQuad(scene, "Left Trigger Fill", {-45.0f, 50.0f, 0.3f}, {2.0f, 7.0f}, m_ActiveColor);
        m_TriggerFill[1] = CreateQuad(scene, "Right Trigger Fill", {45.0f, 50.0f, 0.3f}, {2.0f, 7.0f}, m_ActiveColor);

        m_SystemButtons[0] = CreateCircle(scene, "Share", {-20.0f, 2.0f, 0.3f}, 4.0f, m_IdleColor);
        m_SystemButtons[1] = CreateCircle(scene, "Guide", {0.0f, -16.0f, 0.3f}, 4.5f, m_IdleColor);
        m_SystemButtons[2] = CreateCircle(scene, "Start", {20.0f, 2.0f, 0.3f}, 4.0f, m_IdleColor);

        for (int i = 0; i < 3; ++i) {
            const float y = 62.0f - static_cast<float>(i) * 7.0f;
            m_AccelBars[i] = CreateQuad(scene, "Accelerometer Axis", {82.0f, y, 0.3f}, {2.0f, 4.0f}, {0.2f, 0.75f, 0.95f, 1.0f});
            m_GyroBars[i] = CreateQuad(scene, "Gyro Axis", {-82.0f, y, 0.3f}, {2.0f, 4.0f}, {0.95f, 0.55f, 0.2f, 1.0f});
        }
    }

    void BuildOverlayScene(axiom::Scene& scene) {
        BuildCamera(scene, m_OverlayCamera, true, 180.0f);

        for (int i = 0; i < 7; ++i) {
            const float x = -78.0f + static_cast<float>(i) * 26.0f;
            m_OverlayPulse[i] = CreateCircle(scene, "Loaded Scene Pulse", {x, -78.0f, 0.5f}, 2.4f, {0.25f, 0.95f, 0.65f, 0.45f});
        }
    }

    void UpdateCamera(double dt) {
        auto& input = GetMainInput();
        auto& transform = m_MainCamera.GetComponent<axiom::TransformComponent>();
        auto& camera = m_MainCamera.GetComponent<axiom::CameraComponent>();

        const float moveSpeed = 90.0f * static_cast<float>(dt);
        if (input.IsKeyPressed(axiom::Key::A) || input.IsKeyPressed(axiom::Key::Left))
            transform.Translation.x -= moveSpeed;
        if (input.IsKeyPressed(axiom::Key::D) || input.IsKeyPressed(axiom::Key::Right))
            transform.Translation.x += moveSpeed;
        if (input.IsKeyPressed(axiom::Key::W) || input.IsKeyPressed(axiom::Key::Up))
            transform.Translation.y += moveSpeed;
        if (input.IsKeyPressed(axiom::Key::S) || input.IsKeyPressed(axiom::Key::Down))
            transform.Translation.y -= moveSpeed;

        const glm::vec2 scroll = input.GetMouseScrollDelta();
        if (scroll.y != 0.0f)
            m_CameraSize = glm::clamp(m_CameraSize - scroll.y * 8.0f, 70.0f, 260.0f);
        if (input.IsKeyPressed(axiom::Key::Q))
            m_CameraSize = glm::min(260.0f, m_CameraSize + 80.0f * static_cast<float>(dt));
        if (input.IsKeyPressed(axiom::Key::E))
            m_CameraSize = glm::max(70.0f, m_CameraSize - 80.0f * static_cast<float>(dt));
        if (input.IsKeyPressed(axiom::Key::R)) {
            transform.Translation = glm::vec3(0.0f);
            m_CameraSize = 180.0f;
        }

        camera.OrthographicSize = m_CameraSize;
    }

    void UpdateControllerView() {
        auto& input = GetMainInput();
        const bool connected = input.IsGamepadConnected();

        SetColor(m_ConnectionIndicator, connected ? m_ConnectedColor : m_DisconnectedColor);
        SetColor(m_ControllerBody, connected ? glm::vec4(0.14f, 0.18f, 0.23f, 1.0f) : glm::vec4(0.11f, 0.11f, 0.12f, 1.0f));

        if (!connected) {
            ResetControllerControls();
            return;
        }

        input.SetGamepadSensorEnabled(axiom::GamepadSensors::Accelerometer, true);
        input.SetGamepadSensorEnabled(axiom::GamepadSensors::Gyroscope, true);

        const glm::vec2 leftStick = input.GetGamepadLeftStick();
        const glm::vec2 rightStick = input.GetGamepadRightStick();
        SetTransform(m_LeftStick, {-38.0f + leftStick.x * 7.0f, -7.0f - leftStick.y * 7.0f, 0.3f}, {14.0f, 14.0f});
        SetTransform(m_RightStick, {38.0f + rightStick.x * 7.0f, -7.0f - rightStick.y * 7.0f, 0.3f}, {14.0f, 14.0f});

        UpdateButton(m_DPad[0], input.IsGamepadPressed(axiom::Gamepad::DPadUp));
        UpdateButton(m_DPad[1], input.IsGamepadPressed(axiom::Gamepad::DPadRight));
        UpdateButton(m_DPad[2], input.IsGamepadPressed(axiom::Gamepad::DPadDown));
        UpdateButton(m_DPad[3], input.IsGamepadPressed(axiom::Gamepad::DPadLeft));

        UpdateButton(m_FaceButtons[0], input.IsGamepadPressed(axiom::Gamepad::Cross));
        UpdateButton(m_FaceButtons[1], input.IsGamepadPressed(axiom::Gamepad::Circle));
        UpdateButton(m_FaceButtons[2], input.IsGamepadPressed(axiom::Gamepad::Square));
        UpdateButton(m_FaceButtons[3], input.IsGamepadPressed(axiom::Gamepad::Triangle));

        UpdateButton(m_ShoulderButtons[0], input.IsGamepadPressed(axiom::Gamepad::LeftBumper));
        UpdateButton(m_ShoulderButtons[1], input.IsGamepadPressed(axiom::Gamepad::RightBumper));
        UpdateButton(m_SystemButtons[0], input.IsGamepadPressed(axiom::Gamepad::Share));
        UpdateButton(m_SystemButtons[1], input.IsGamepadPressed(axiom::Gamepad::Guide));
        UpdateButton(m_SystemButtons[2], input.IsGamepadPressed(axiom::Gamepad::Start));
        UpdateButton(m_Touchpad, input.IsGamepadPressed(axiom::Gamepad::Touchpad));
        UpdateTouchpadFinger();

        const float leftTrigger = glm::clamp(input.GetGamepadLeftTrigger(), 0.0f, 1.0f);
        const float rightTrigger = glm::clamp(input.GetGamepadRightTrigger(), 0.0f, 1.0f);
        UpdateTrigger(m_TriggerFill[0], -45.0f, leftTrigger);
        UpdateTrigger(m_TriggerFill[1], 45.0f, rightTrigger);

        const glm::vec3 ledColor = glm::clamp(glm::vec3(leftStick.x * 0.5f + 0.5f, rightStick.x * 0.5f + 0.5f, rightTrigger), 0.0f, 1.0f);
        SetColor(m_LedSwatch, glm::vec4(ledColor, 1.0f));
        input.SetGamepadLED(
            static_cast<uint8_t>(ledColor.r * 255.0f),
            static_cast<uint8_t>(ledColor.g * 255.0f),
            static_cast<uint8_t>(ledColor.b * 255.0f)
        );

        if (input.IsGamepadPressed(axiom::Gamepad::Cross))
            input.RumbleGamepad(0xffff, 0x5000, 40);
        if (leftTrigger > 0.05f || rightTrigger > 0.05f)
            input.RumbleGamepadTriggers(static_cast<uint16_t>(leftTrigger * 0xffff), static_cast<uint16_t>(rightTrigger * 0xffff), 40);

        float accel[3] {};
        float gyro[3] {};
        if (input.GetGamepadSensorData(axiom::GamepadSensors::Accelerometer, accel, 3))
            UpdateSensorBars(m_AccelBars, 82.0f, accel, 0.08f);
        if (input.GetGamepadSensorData(axiom::GamepadSensors::Gyroscope, gyro, 3))
            UpdateSensorBars(m_GyroBars, -82.0f, gyro, 1.8f);
    }

    void ResetControllerControls() {
        SetTransform(m_LeftStick, {-38.0f, -7.0f, 0.3f}, {14.0f, 14.0f});
        SetTransform(m_RightStick, {38.0f, -7.0f, 0.3f}, {14.0f, 14.0f});
        for (axiom::Entity entity : m_DPad)
            SetColor(entity, m_IdleColor);
        for (axiom::Entity entity : m_FaceButtons)
            SetColor(entity, m_IdleColor);
        for (axiom::Entity entity : m_ShoulderButtons)
            SetColor(entity, m_IdleColor);
        for (axiom::Entity entity : m_SystemButtons)
            SetColor(entity, m_IdleColor);
        SetColor(m_Touchpad, {0.24f, 0.27f, 0.32f, 1.0f});
        SetColor(m_TouchpadFinger, {0.18f, 0.95f, 0.62f, 0.0f});
        UpdateTrigger(m_TriggerFill[0], -45.0f, 0.0f);
        UpdateTrigger(m_TriggerFill[1], 45.0f, 0.0f);
    }

    void UpdateButton(axiom::Entity entity, bool pressed) {
        SetColor(entity, pressed ? m_ActiveColor : m_IdleColor);
    }

    void UpdateTouchpadFinger() {
        auto& input = GetMainInput();
        bool down = false;
        float x = 0.0f;
        float y = 0.0f;
        float pressure = 0.0f;

        if (input.GetGamepadTouchpadCount() <= 0 ||
            input.GetGamepadTouchpadFingerCount() <= 0 ||
            !input.GetGamepadTouchpadFinger(0, 0, down, x, y, pressure) ||
            !down) {
            SetColor(m_TouchpadFinger, {0.18f, 0.95f, 0.62f, 0.0f});
            return;
        }

        const float touchX = -17.0f + glm::clamp(x, 0.0f, 1.0f) * 34.0f;
        const float touchY = 27.0f - glm::clamp(y, 0.0f, 1.0f) * 18.0f;
        const float radius = 2.2f + glm::clamp(pressure, 0.0f, 1.0f) * 3.0f;
        SetTransform(m_TouchpadFinger, {touchX, touchY, 0.35f}, {radius * 2.0f, radius * 2.0f});
        SetColor(m_TouchpadFinger, {0.18f, 0.95f, 0.62f, 0.95f});
    }

    void UpdateTrigger(axiom::Entity entity, float centerX, float value) {
        const float width = 2.0f + value * 30.0f;
        SetTransform(entity, {centerX - 15.0f + width * 0.5f, 50.0f, 0.3f}, {width, 7.0f});
    }

    void UpdateSensorBars(const std::array<axiom::Entity, 3>& bars, float centerX, const float* values, float scale) {
        for (int i = 0; i < 3; ++i) {
            const float y = 62.0f - static_cast<float>(i) * 7.0f;
            const float width = glm::clamp(std::abs(values[i]) * scale, 2.0f, 28.0f);
            const float direction = values[i] >= 0.0f ? 1.0f : -1.0f;
            SetTransform(bars[i], {centerX + direction * width * 0.5f, y, 0.3f}, {width, 4.0f});
        }
    }

    void UpdateOverlayScene() {
        const float t = static_cast<float>(axiom::Time::GetTime());
        for (size_t i = 0; i < m_OverlayPulse.size(); ++i) {
            const float pulse = 0.5f + 0.5f * std::sin(t * 2.0f + static_cast<float>(i));
            SetColor(m_OverlayPulse[i], {0.2f, 0.8f + pulse * 0.2f, 0.55f, 0.25f + pulse * 0.35f});
        }
    }

private:
    std::shared_ptr<axiom::Render2DSystem> m_Render2DSystem;

    axiom::Entity m_MainCamera;
    axiom::Entity m_OverlayCamera;
    float m_CameraSize = 180.0f;

    axiom::Entity m_ControllerBody;
    axiom::Entity m_ConnectionIndicator;
    axiom::Entity m_Touchpad;
    axiom::Entity m_TouchpadFinger;
    axiom::Entity m_LedSwatch;
    axiom::Entity m_LeftStickBase;
    axiom::Entity m_LeftStick;
    axiom::Entity m_RightStickBase;
    axiom::Entity m_RightStick;
    std::array<axiom::Entity, 4> m_DPad;
    std::array<axiom::Entity, 4> m_FaceButtons;
    std::array<axiom::Entity, 2> m_ShoulderButtons;
    std::array<axiom::Entity, 2> m_TriggerFill;
    std::array<axiom::Entity, 3> m_SystemButtons;
    std::array<axiom::Entity, 3> m_AccelBars;
    std::array<axiom::Entity, 3> m_GyroBars;
    std::array<axiom::Entity, 7> m_OverlayPulse;

    const glm::vec4 m_IdleColor {0.32f, 0.36f, 0.42f, 1.0f};
    const glm::vec4 m_ActiveColor {0.18f, 0.95f, 0.62f, 1.0f};
    const glm::vec4 m_ConnectedColor {0.18f, 0.95f, 0.62f, 1.0f};
    const glm::vec4 m_DisconnectedColor {0.95f, 0.2f, 0.24f, 1.0f};
};

namespace axiom {

Application* CreateApplication() {
    return new Testbed();
}

} // namespace axiom

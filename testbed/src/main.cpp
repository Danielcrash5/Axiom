/**
 * @file testbed/src/main.cpp
 * @brief Implements the main module.
 */

// main.cpp
#include <axiom/Axiom.h>

#include "Panels/InspectorPanel.h"
#include "Panels/SceneHierarchyPanel.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <axiom/renderer/passes/QuadRenderPass.h>
#include <string>

/**
 * @brief Provides the testbed functionality.
 * @tparam Testbed Template type used by this type.
 */
class Testbed : public axiom::Application {
  public:
    /**
     * @brief Creates an instance of testbed.
     */
    Testbed() : Application("Axiom Controller Test") {}

  protected:
    /**
     * @brief Handles the init callback.
     */
    void onInit() override {
        axiom::VFS::mountPath("game://", AXIOM_GAME_ASSET_PATH);

        auto& renderer = getRenderer();
        const auto format = renderer.surfaceFormat(
            renderer.mainSurface(), getWidth(), getHeigth());
        if (!format) {
            AXIOM_ERROR("Testbed: Swapchain-Format fuer den Quad nicht verfuegbar");
            return;
        }

        auto quadPass =
            std::make_unique<axiom::renderer::passes::QuadRenderPass>();
        if (auto result = quadPass->initialize(
                renderer, renderer.mainSurface(), *format); !result) {
            AXIOM_ERROR("Testbed: Quad-Renderpass konnte nicht initialisiert werden");
            return;
        }
        if (m_ImGuiLayer) {
            renderer.addPassFirst(std::move(quadPass));
        } else {
            renderer.addPass(std::move(quadPass));
        }

        m_render2DSystem = registerSystem<axiom::Render2DSystem>(
            renderer, getMainViewId(), getWidth(), getHeigth());
        buildControllerScene(createScene());

        /*std::shared_ptr<testbed::InspectorPanel> inspectorPanel =
        std::make_shared<testbed::InspectorPanel>();

        AddImGuiPanel(std::make_shared<testbed::SceneHierarchyPanel>(m_Scenes,
        inspectorPanel)); AddImGuiPanel(inspectorPanel);*/
    }

    /**
     * @brief Handles the update callback.
     * @param dt The dt used by this operation.
     */
    void onUpdate(double dt) override {
        updateCamera(dt);
        updateControllerView();
    }

  private:
    /**
     * @brief Creates quad.
     * @param scene The scene used by this operation.
     * @param name The name used by this operation.
     * @param position The position used by this operation.
     * @param size The size used by this operation.
     * @param color The color used by this operation.
     * @return The result of the operation.
     */
    axiom::Entity createQuad(axiom::Scene &scene, const std::string &name,
                             const glm::vec3 &position, const glm::vec2 &size,
                             const glm::vec4 &color) {
        axiom::Entity entity = scene.createEntity(name);
        auto &transform = entity.getComponent<axiom::TransformComponent>();
        transform.Translation = position;
        transform.Scale = glm::vec3(size, 1.0f);
        entity.addComponent<axiom::QuadRendererComponent>(color);
        return entity;
    }

    /**
     * @brief Creates circle.
     * @param scene The scene used by this operation.
     * @param name The name used by this operation.
     * @param position The position used by this operation.
     * @param radius The radius used by this operation.
     * @param color The color used by this operation.
     * @param thickness The thickness used by this operation.
     * @return The result of the operation.
     */
    axiom::Entity createCircle(axiom::Scene &scene, const std::string &name,
                               const glm::vec3 &position, float radius,
                               const glm::vec4 &color, float thickness = 1.0f) {
        axiom::Entity entity = scene.createEntity(name);
        auto &transform = entity.getComponent<axiom::TransformComponent>();
        transform.Translation = position;
        transform.Scale = glm::vec3(radius * 2.0f, radius * 2.0f, 1.0f);
        entity.addComponent<axiom::CircleRendererComponent>(color, thickness);
        return entity;
    }

    /**
     * @brief Sets color.
     * @param entity The entity used by this operation.
     * @param color The color used by this operation.
     */
    void setColor(axiom::Entity entity, const glm::vec4 &color) {
        if (entity.hasComponent<axiom::QuadRendererComponent>())
            entity.getComponent<axiom::QuadRendererComponent>().Color = color;
        if (entity.hasComponent<axiom::CircleRendererComponent>())
            entity.getComponent<axiom::CircleRendererComponent>().Color = color;
    }

    /**
     * @brief Sets transform.
     * @param entity The entity used by this operation.
     * @param position The position used by this operation.
     * @param size The size used by this operation.
     */
    void setTransform(axiom::Entity entity, const glm::vec3 &position,
                      const glm::vec2 &size) {
        auto &transform = entity.getComponent<axiom::TransformComponent>();
        transform.Translation = position;
        transform.Scale = glm::vec3(size, 1.0f);
    }

    /**
     * @brief Builds camera.
     * @param scene The scene used by this operation.
     * @param cameraEntity The camera entity used by this operation.
     * @param primary The primary used by this operation.
     * @param size The size used by this operation.
     */
    void buildCamera(axiom::Scene &scene, axiom::Entity &cameraEntity,
                     bool primary, float size) {
        cameraEntity =
            scene.createEntity(primary ? "Primary Camera" : "Overlay Camera");
        auto &camera =
            cameraEntity.addComponent<axiom::CameraComponent>(primary);
        camera.OrthographicSize = size;
    }

    /**
     * @brief Builds controller scene.
     * @param scene The scene used by this operation.
     */
    void buildControllerScene(axiom::Scene &scene) {
        buildCamera(scene, m_mainCamera, true, m_cameraSize);

        m_connectionIndicator =
            createQuad(scene, "Connection Indicator", {-85.0f, 72.0f, 0.2f},
                       {18.0f, 8.0f}, m_disconnectedColor);
        m_controllerBody =
            createQuad(scene, "Controller Body", {0.0f, 0.0f, 0.0f},
                       {130.0f, 58.0f}, {0.12f, 0.15f, 0.18f, 1.0f});
        m_touchpad = createQuad(scene, "Touchpad", {0.0f, 18.0f, 0.2f},
                                {34.0f, 18.0f}, {0.24f, 0.27f, 0.32f, 1.0f});
        m_touchpadFinger =
            createCircle(scene, "Touchpad Finger", {0.0f, 18.0f, 0.35f}, 2.2f,
                         {0.18f, 0.95f, 0.62f, 0.0f});
        m_ledSwatch = createQuad(scene, "LED Swatch", {0.0f, -39.0f, 0.2f},
                                 {42.0f, 5.0f}, {0.2f, 0.3f, 0.8f, 1.0f});

        m_leftStickBase =
            createCircle(scene, "Left Stick Base", {-38.0f, -7.0f, 0.2f}, 13.0f,
                         {0.08f, 0.09f, 0.11f, 1.0f});
        m_leftStick = createCircle(scene, "Left Stick", {-38.0f, -7.0f, 0.3f},
                                   7.0f, {0.38f, 0.46f, 0.56f, 1.0f});
        m_rightStickBase =
            createCircle(scene, "Right Stick Base", {38.0f, -7.0f, 0.2f}, 13.0f,
                         {0.08f, 0.09f, 0.11f, 1.0f});
        m_rightStick = createCircle(scene, "Right Stick", {38.0f, -7.0f, 0.3f},
                                    7.0f, {0.38f, 0.46f, 0.56f, 1.0f});

        m_dPad[0] = createQuad(scene, "DPad Up", {-66.0f, 15.0f, 0.3f},
                               {8.0f, 12.0f}, m_idleColor);
        m_dPad[1] = createQuad(scene, "DPad Right", {-56.0f, 5.0f, 0.3f},
                               {12.0f, 8.0f}, m_idleColor);
        m_dPad[2] = createQuad(scene, "DPad Down", {-66.0f, -5.0f, 0.3f},
                               {8.0f, 12.0f}, m_idleColor);
        m_dPad[3] = createQuad(scene, "DPad Left", {-76.0f, 5.0f, 0.3f},
                               {12.0f, 8.0f}, m_idleColor);

        m_faceButtons[0] = createCircle(scene, "Cross", {66.0f, -8.0f, 0.3f},
                                        5.0f, m_idleColor);
        m_faceButtons[1] = createCircle(scene, "Circle", {77.0f, 3.0f, 0.3f},
                                        5.0f, m_idleColor);
        m_faceButtons[2] = createCircle(scene, "Square", {55.0f, 3.0f, 0.3f},
                                        5.0f, m_idleColor);
        m_faceButtons[3] = createCircle(scene, "Triangle", {66.0f, 14.0f, 0.3f},
                                        5.0f, m_idleColor);

        m_shoulderButtons[0] =
            createQuad(scene, "Left Shoulder", {-45.0f, 38.0f, 0.3f},
                       {32.0f, 8.0f}, m_idleColor);
        m_shoulderButtons[1] =
            createQuad(scene, "Right Shoulder", {45.0f, 38.0f, 0.3f},
                       {32.0f, 8.0f}, m_idleColor);
        m_triggerFill[0] =
            createQuad(scene, "Left Trigger Fill", {-45.0f, 50.0f, 0.3f},
                       {2.0f, 7.0f}, m_activeColor);
        m_triggerFill[1] =
            createQuad(scene, "Right Trigger Fill", {45.0f, 50.0f, 0.3f},
                       {2.0f, 7.0f}, m_activeColor);

        m_systemButtons[0] = createCircle(scene, "Share", {-20.0f, 2.0f, 0.3f},
                                          4.0f, m_idleColor);
        m_systemButtons[1] = createCircle(scene, "Guide", {0.0f, -16.0f, 0.3f},
                                          4.5f, m_idleColor);
        m_systemButtons[2] = createCircle(scene, "Start", {20.0f, 2.0f, 0.3f},
                                          4.0f, m_idleColor);

        for (int i = 0; i < 3; ++i) {
            const float y = 62.0f - static_cast<float>(i) * 7.0f;
            m_accelBars[i] =
                createQuad(scene, "Accelerometer Axis", {82.0f, y, 0.3f},
                           {2.0f, 4.0f}, {0.2f, 0.75f, 0.95f, 1.0f});
            m_gyroBars[i] =
                createQuad(scene, "Gyro Axis", {-82.0f, y, 0.3f}, {2.0f, 4.0f},
                           {0.95f, 0.55f, 0.2f, 1.0f});
        }
    }

    /**
     * @brief Builds overlay scene.
     * @param scene The scene used by this operation.
     */
    void buildOverlayScene(axiom::Scene &scene) {
        buildCamera(scene, m_overlayCamera, true, 180.0f);

        for (int i = 0; i < 7; ++i) {
            const float x = -78.0f + static_cast<float>(i) * 26.0f;
            m_overlayPulse[i] =
                createCircle(scene, "Loaded Scene Pulse", {x, -78.0f, 0.5f},
                             2.4f, {0.25f, 0.95f, 0.65f, 0.45f});
        }
    }

    /**
     * @brief Updates camera.
     * @param dt The dt used by this operation.
     */
    void updateCamera(double dt) {
        auto &input = GetMainInput();
        auto &transform =
            m_mainCamera.getComponent<axiom::TransformComponent>();
        auto &camera = m_mainCamera.getComponent<axiom::CameraComponent>();

        const float moveSpeed = 90.0f * static_cast<float>(dt);
        if (input.isKeyPressed(axiom::key::a) ||
            input.isKeyPressed(axiom::key::left))
            transform.Translation.x -= moveSpeed;
        if (input.isKeyPressed(axiom::key::d) ||
            input.isKeyPressed(axiom::key::right))
            transform.Translation.x += moveSpeed;
        if (input.isKeyPressed(axiom::key::w) ||
            input.isKeyPressed(axiom::key::up))
            transform.Translation.y += moveSpeed;
        if (input.isKeyPressed(axiom::key::s) ||
            input.isKeyPressed(axiom::key::down))
            transform.Translation.y -= moveSpeed;

        const glm::vec2 scroll = input.getMouseScrollDelta();
        if (scroll.y != 0.0f)
            m_cameraSize =
                glm::clamp(m_cameraSize - scroll.y * 8.0f, 70.0f, 260.0f);
        if (input.isKeyPressed(axiom::key::q))
            m_cameraSize =
                glm::min(260.0f, m_cameraSize + 80.0f * static_cast<float>(dt));
        if (input.isKeyPressed(axiom::key::e))
            m_cameraSize =
                glm::max(70.0f, m_cameraSize - 80.0f * static_cast<float>(dt));
        if (input.isKeyPressed(axiom::key::r)) {
            transform.Translation = glm::vec3(0.0f);
            m_cameraSize = 180.0f;
        }

        camera.OrthographicSize = m_cameraSize;
    }

    /**
     * @brief Updates controller view.
     */
    void updateControllerView() {
        auto &input = GetMainInput();
        const bool connected = input.isGamepadConnected();

        setColor(m_connectionIndicator,
                 connected ? m_connectedColor : m_disconnectedColor);
        setColor(m_controllerBody, connected
                                       ? glm::vec4(0.14f, 0.18f, 0.23f, 1.0f)
                                       : glm::vec4(0.11f, 0.11f, 0.12f, 1.0f));

        if (!connected) {
            resetControllerControls();
            return;
        }

        input.setGamepadSensorEnabled(axiom::gamepad_sensors::accelerometer,
                                      true);
        input.setGamepadSensorEnabled(axiom::gamepad_sensors::gyroscope, true);

        const glm::vec2 leftStick = input.getGamepadLeftStick();
        const glm::vec2 rightStick = input.getGamepadRightStick();
        setTransform(
            m_leftStick,
            {-38.0f + leftStick.x * 7.0f, -7.0f - leftStick.y * 7.0f, 0.3f},
            {14.0f, 14.0f});
        setTransform(
            m_rightStick,
            {38.0f + rightStick.x * 7.0f, -7.0f - rightStick.y * 7.0f, 0.3f},
            {14.0f, 14.0f});

        updateButton(m_dPad[0], input.isGamepadPressed(axiom::gamepad::dPadUp));
        updateButton(m_dPad[1],
                     input.isGamepadPressed(axiom::gamepad::dPadRight));
        updateButton(m_dPad[2],
                     input.isGamepadPressed(axiom::gamepad::dPadDown));
        updateButton(m_dPad[3],
                     input.isGamepadPressed(axiom::gamepad::dPadLeft));

        updateButton(m_faceButtons[0],
                     input.isGamepadPressed(axiom::gamepad::cross));
        updateButton(m_faceButtons[1],
                     input.isGamepadPressed(axiom::gamepad::circle));
        updateButton(m_faceButtons[2],
                     input.isGamepadPressed(axiom::gamepad::square));
        updateButton(m_faceButtons[3],
                     input.isGamepadPressed(axiom::gamepad::triangle));

        updateButton(m_shoulderButtons[0],
                     input.isGamepadPressed(axiom::gamepad::leftBumper));
        updateButton(m_shoulderButtons[1],
                     input.isGamepadPressed(axiom::gamepad::rightBumper));
        updateButton(m_systemButtons[0],
                     input.isGamepadPressed(axiom::gamepad::back));
        updateButton(m_systemButtons[1],
                     input.isGamepadPressed(axiom::gamepad::guide));
        updateButton(m_systemButtons[2],
                     input.isGamepadPressed(axiom::gamepad::start));
        updateButton(m_touchpad,
                     input.isGamepadPressed(axiom::gamepad::touchpad));

        updateButton(m_leftStick,
                     input.isGamepadPressed(axiom::gamepad::leftStick));
        updateButton(m_rightStick,
                     input.isGamepadPressed(axiom::gamepad::rightStick));
        updateTouchpadFinger();

        const float leftTrigger =
            glm::clamp(input.getGamepadLeftTrigger(), 0.0f, 1.0f);
        const float rightTrigger =
            glm::clamp(input.getGamepadRightTrigger(), 0.0f, 1.0f);
        updateTrigger(m_triggerFill[0], -45.0f, leftTrigger);
        updateTrigger(m_triggerFill[1], 45.0f, rightTrigger);

        const glm::vec3 ledColor =
            glm::clamp(glm::vec3(leftStick.x * 0.5f + 0.5f,
                                 rightStick.x * 0.5f + 0.5f, rightTrigger),
                       0.0f, 1.0f);
        setColor(m_ledSwatch, glm::vec4(ledColor, 1.0f));
        input.setGamepadLed(static_cast<uint8_t>(ledColor.r * 255.0f),
                            static_cast<uint8_t>(ledColor.g * 255.0f),
                            static_cast<uint8_t>(ledColor.b * 255.0f));

        if (input.isGamepadPressed(axiom::gamepad::cross))
            input.rumbleGamepad(0xffff, 0x5000, 40);
        if (leftTrigger > 0.05f || rightTrigger > 0.05f)
            input.rumbleGamepadTriggers(
                static_cast<uint16_t>(leftTrigger * 0xffff),
                static_cast<uint16_t>(rightTrigger * 0xffff), 40);

        float accel[3]{};
        float gyro[3]{};
        if (input.getGamepadSensorData(axiom::gamepad_sensors::accelerometer,
                                       accel, 3))
            updateSensorBars(m_accelBars, 82.0f, accel, 0.08f);
        if (input.getGamepadSensorData(axiom::gamepad_sensors::gyroscope, gyro,
                                       3))
            updateSensorBars(m_gyroBars, -82.0f, gyro, 1.8f);
    }

    /**
     * @brief Resets controller controls.
     */
    void resetControllerControls() {
        setTransform(m_leftStick, {-38.0f, -7.0f, 0.3f}, {14.0f, 14.0f});
        setTransform(m_rightStick, {38.0f, -7.0f, 0.3f}, {14.0f, 14.0f});
        for (axiom::Entity entity : m_dPad)
            setColor(entity, m_idleColor);
        for (axiom::Entity entity : m_faceButtons)
            setColor(entity, m_idleColor);
        for (axiom::Entity entity : m_shoulderButtons)
            setColor(entity, m_idleColor);
        for (axiom::Entity entity : m_systemButtons)
            setColor(entity, m_idleColor);
        setColor(m_touchpad, {0.24f, 0.27f, 0.32f, 1.0f});
        setColor(m_touchpadFinger, {0.18f, 0.95f, 0.62f, 0.0f});
        updateTrigger(m_triggerFill[0], -45.0f, 0.0f);
        updateTrigger(m_triggerFill[1], 45.0f, 0.0f);
    }

    /**
     * @brief Updates button.
     * @param entity The entity used by this operation.
     * @param pressed The pressed used by this operation.
     */
    void updateButton(axiom::Entity entity, bool pressed) {
        setColor(entity, pressed ? m_activeColor : m_idleColor);
    }

    /**
     * @brief Updates touchpad finger.
     */
    void updateTouchpadFinger() {
        auto &input = GetMainInput();
        bool down = false;
        float x = 0.0f;
        float y = 0.0f;
        float pressure = 0.0f;

        if (input.getGamepadTouchpadCount() <= 0 ||
            input.getGamepadTouchpadFingerCount() <= 0 ||
            !input.getGamepadTouchpadFinger(0, 0, down, x, y, pressure) ||
            !down) {
            setColor(m_touchpadFinger, {0.18f, 0.95f, 0.62f, 0.0f});
            return;
        }

        const float touchX = -17.0f + glm::clamp(x, 0.0f, 1.0f) * 34.0f;
        const float touchY = 27.0f - glm::clamp(y, 0.0f, 1.0f) * 18.0f;
        const float radius = 2.2f + glm::clamp(pressure, 0.0f, 1.0f) * 3.0f;
        setTransform(m_touchpadFinger, {touchX, touchY, 0.35f},
                     {radius * 2.0f, radius * 2.0f});
        setColor(m_touchpadFinger, {0.18f, 0.95f, 0.62f, 0.95f});
    }

    /**
     * @brief Updates trigger.
     * @param entity The entity used by this operation.
     * @param centerX The center x used by this operation.
     * @param value The value used by this operation.
     */
    void updateTrigger(axiom::Entity entity, float centerX, float value) {
        const float width = 2.0f + value * 30.0f;
        setTransform(entity, {centerX - 15.0f + width * 0.5f, 50.0f, 0.3f},
                     {width, 7.0f});
    }

    /**
     * @brief Updates sensor bars.
     * @param bars The bars used by this operation.
     * @param centerX The center x used by this operation.
     * @param values The values used by this operation.
     * @param scale The scale used by this operation.
     */
    void updateSensorBars(const std::array<axiom::Entity, 3> &bars,
                          float centerX, const float *values, float scale) {
        for (int i = 0; i < 3; ++i) {
            const float y = 62.0f - static_cast<float>(i) * 7.0f;
            const float width =
                glm::clamp(std::abs(values[i]) * scale, 2.0f, 28.0f);
            const float direction = values[i] >= 0.0f ? 1.0f : -1.0f;
            setTransform(bars[i], {centerX + direction * width * 0.5f, y, 0.3f},
                         {width, 4.0f});
        }
    }

    /**
     * @brief Updates overlay scene.
     */
    void updateOverlayScene() {
        const float t = static_cast<float>(axiom::Time::getTime());
        for (size_t i = 0; i < m_overlayPulse.size(); ++i) {
            const float pulse =
                0.5f + 0.5f * std::sin(t * 2.0f + static_cast<float>(i));
            setColor(m_overlayPulse[i],
                     {0.2f, 0.8f + pulse * 0.2f, 0.55f, 0.25f + pulse * 0.35f});
        }
    }

  private:
    /** @brief Stores the render2 d system. */
    std::shared_ptr<axiom::Render2DSystem> m_render2DSystem;

    /** @brief Stores the main camera. */
    axiom::Entity m_mainCamera;
    /** @brief Stores the overlay camera. */
    axiom::Entity m_overlayCamera;
    /** @brief Stores the camera size. */
    float m_cameraSize = 180.0f;

    /** @brief Stores the controller body. */
    axiom::Entity m_controllerBody;
    /** @brief Stores the connection indicator. */
    axiom::Entity m_connectionIndicator;
    /** @brief Stores the touchpad. */
    axiom::Entity m_touchpad;
    /** @brief Stores the touchpad finger. */
    axiom::Entity m_touchpadFinger;
    /** @brief Stores the led swatch. */
    axiom::Entity m_ledSwatch;
    /** @brief Stores the left stick base. */
    axiom::Entity m_leftStickBase;
    /** @brief Stores the left stick. */
    axiom::Entity m_leftStick;
    /** @brief Stores the right stick base. */
    axiom::Entity m_rightStickBase;
    /** @brief Stores the right stick. */
    axiom::Entity m_rightStick;
    /** @brief Stores the d pad. */
    std::array<axiom::Entity, 4> m_dPad;
    /** @brief Stores the face buttons. */
    std::array<axiom::Entity, 4> m_faceButtons;
    /** @brief Stores the shoulder buttons. */
    std::array<axiom::Entity, 2> m_shoulderButtons;
    /** @brief Stores the trigger fill. */
    std::array<axiom::Entity, 2> m_triggerFill;
    /** @brief Stores the system buttons. */
    std::array<axiom::Entity, 3> m_systemButtons;
    /** @brief Stores the accel bars. */
    std::array<axiom::Entity, 3> m_accelBars;
    /** @brief Stores the gyro bars. */
    std::array<axiom::Entity, 3> m_gyroBars;
    /** @brief Stores the overlay pulse. */
    std::array<axiom::Entity, 7> m_overlayPulse;

    /** @brief Stores the idle color. */
    const glm::vec4 m_idleColor{0.32f, 0.36f, 0.42f, 1.0f};
    /** @brief Stores the active color. */
    const glm::vec4 m_activeColor{0.18f, 0.95f, 0.62f, 1.0f};
    /** @brief Stores the connected color. */
    const glm::vec4 m_connectedColor{0.18f, 0.95f, 0.62f, 1.0f};
    /** @brief Stores the disconnected color. */
    const glm::vec4 m_disconnectedColor{0.95f, 0.2f, 0.24f, 1.0f};
};

namespace axiom {

Application *createApplication() { return new Testbed(); }

} // namespace axiom

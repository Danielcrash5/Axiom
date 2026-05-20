// main.cpp
#include <axiom/Axiom.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

namespace {
    std::shared_ptr<axiom::Texture2D> LoadGameTexture(const std::string& virtualPath) {
        auto info = axiom::TexturePresets::Sprite();
        return axiom::AssetManager::Get<axiom::Texture2D>(virtualPath, info);
    }

    std::shared_ptr<axiom::Material> CreateTexturedOverrideMaterial() {
        std::shared_ptr<axiom::Shader> shader = axiom::AssetManager::Get<axiom::Shader>("game://shaders/Textured.glsl");

        return std::make_shared<axiom::Material>(shader);
    }

    std::shared_ptr<axiom::Material> CreateSkinnedColorOverrideMaterial() {
        std::shared_ptr<axiom::Shader> shader = axiom::AssetManager::Get<axiom::Shader>("game://shaders/SkinnedColor.glsl");

        return std::make_shared<axiom::Material>(shader);
    }

    axiom::SkinnedMesh2D CreateRibbonSkinnedMesh(const std::shared_ptr<axiom::Texture2D>& texture) {
        axiom::SkinnedMesh2D mesh;
        mesh.Texture = texture;

        const int columns = 12;
        mesh.Vertices.reserve((columns + 1) * 2);
        mesh.Indices.reserve(columns * 6);

        for (int x = 0; x <= columns; ++x) {
            const float u = static_cast<float>(x) / static_cast<float>(columns);
            const float px = -0.5f + u;
            const float weightBone1 = glm::smoothstep(0.25f, 1.0f, u);
            const float weightBone0 = 1.0f - weightBone1;

            const glm::vec4 boneIndices = {0.0f, 1.0f, 0.0f, 0.0f};
            const glm::vec4 boneWeights = {weightBone0, weightBone1, 0.0f, 0.0f};

            mesh.Vertices.push_back({
                { px, -0.5f, 0.0f },
                { 1.0f, 0.4f + 0.5f * u, 0.4f, 1.0f },
                { u, 0.0f },
                boneIndices,
                boneWeights
                                    });
            mesh.Vertices.push_back({
                { px, 0.5f, 0.0f },
                { 0.4f, 0.7f, 1.0f - 0.4f * u, 1.0f },
                { u, 1.0f },
                boneIndices,
                boneWeights
                                    });
        }

        for (int x = 0; x < columns; ++x) {
            const uint32_t i0 = static_cast<uint32_t>(x * 2);
            const uint32_t i1 = i0 + 1;
            const uint32_t i2 = i0 + 2;
            const uint32_t i3 = i0 + 3;

            mesh.Indices.push_back(i0);
            mesh.Indices.push_back(i2);
            mesh.Indices.push_back(i3);
            mesh.Indices.push_back(i3);
            mesh.Indices.push_back(i1);
            mesh.Indices.push_back(i0);
        }

        return mesh;
    }
}

class Testbed : public axiom::Application {
public:
    Testbed() :
        Application("testbed") {
    }

protected:
    void OnInit() override {
        axiom::Renderer2D::Init();
        ApplyDebugCamera();

        axiom::VFS::MountPath("game://", AXIOM_GAME_ASSET_PATH);

        m_Texture = LoadGameTexture("game://textures/Purple/texture_01.png");
        m_SkinnedTexture = LoadGameTexture("game://textures/Orange/texture_01.png");
        m_Sprite = axiom::Sprite(m_Texture, {0.0f, 0.0f}, {1.0f, 1.0f});
        m_OverrideMaterial = CreateTexturedOverrideMaterial();
        m_SkinnedDebugMaterial = CreateSkinnedColorOverrideMaterial();

        m_SkinnedMesh = CreateRibbonSkinnedMesh(m_SkinnedTexture);

        m_Pose.BoneCount = 2;
        m_Pose.BoneTransforms[0] = glm::mat4(1.0f);
        m_Pose.BoneTransforms[1] = glm::mat4(1.0f);

        BuildEcsScene();
        RegisterSystem<axiom::Render2DSystem>();
    }

    void OnUpdate(double dt) override {
        const float moveSpeed = 90.0f / m_CameraZoom;
        const float zoomSpeed = 1.5f;
        bool cameraChanged = false;

        auto& input = GetMainInput();
        const glm::vec2 mouseScroll = input.GetMouseScrollDelta();

        if (mouseScroll.y != 0.0f) {
            m_CameraZoom = glm::clamp(m_CameraZoom + mouseScroll.y * 0.15f, 0.2f, 16.0f);
            cameraChanged = true;
        }

        if (input.IsKeyPressed(axiom::Key::A) || input.IsKeyPressed(axiom::Key::Left)) {
            m_CameraPosition.x -= moveSpeed * static_cast<float>(dt);
            cameraChanged = true;
        }
        if (input.IsKeyPressed(axiom::Key::D) || input.IsKeyPressed(axiom::Key::Right)) {
            m_CameraPosition.x += moveSpeed * static_cast<float>(dt);
            cameraChanged = true;
        }
        if (input.IsKeyPressed(axiom::Key::W) || input.IsKeyPressed(axiom::Key::Up)) {
            m_CameraPosition.y += moveSpeed * static_cast<float>(dt);
            cameraChanged = true;
        }
        if (input.IsKeyPressed(axiom::Key::S) || input.IsKeyPressed(axiom::Key::Down)) {
            m_CameraPosition.y -= moveSpeed * static_cast<float>(dt);
            cameraChanged = true;
        }
        if (input.IsKeyPressed(axiom::Key::Q)) {
            m_CameraZoom = glm::max(0.2f, m_CameraZoom - zoomSpeed * static_cast<float>(dt));
            cameraChanged = true;
        }
        if (input.IsKeyPressed(axiom::Key::E)) {
            m_CameraZoom = glm::min(16.0f, m_CameraZoom + zoomSpeed * static_cast<float>(dt));
            cameraChanged = true;
        }
        if (input.IsKeyPressed(axiom::Key::R)) {
            m_CameraPosition = glm::vec3(0.0f);
            m_CameraZoom = 1.0f;
            cameraChanged = true;
        }

        if (cameraChanged)
            ApplyDebugCamera();
    }

    void OnRender(double alpha) override {
        (void)alpha;

        const float t = static_cast<float>(axiom::Time::GetTime());
        const float swing = sinf(t * 2.6f) * 1.25f;
        const float bendLift = cosf(t * 1.9f) * 0.12f;
        const float pivotX = 0.15f;

        m_EcsColoredQuad.GetComponent<axiom::TransformComponent>().Rotation.z =
            glm::radians(22.0f) + sinf(t * 1.4f) * 0.35f;
        m_EcsTexturedSprite.GetComponent<axiom::TransformComponent>().Rotation.z =
            glm::radians(-12.0f) + cosf(t * 1.1f) * 0.2f;
        m_EcsCircle.GetComponent<axiom::TransformComponent>().Translation.y =
            -20.0f + sinf(t * 1.7f) * 9.0f;

        m_Pose.BoneTransforms[0] = glm::mat4(1.0f);
        m_Pose.BoneTransforms[1] =
            glm::translate(glm::mat4(1.0f), glm::vec3(pivotX, bendLift, 0.0f)) *
            glm::rotate(glm::mat4(1.0f), swing, glm::vec3(0.0f, 0.0f, 1.0f)) *
            glm::translate(glm::mat4(1.0f), glm::vec3(-pivotX, 0.0f, 0.0f));

        axiom::Renderer::BeginScene(ViewProjection, {});

        axiom::Renderer2D::BeginScene();

        glm::mat4 overrideQuadTransform =
            glm::translate(glm::mat4(1.0f), glm::vec3(5.0f, 20.0f, 0.2f)) *
            glm::scale(glm::mat4(1.0f), glm::vec3(34.0f, 34.0f, 1.0f));
        axiom::Renderer2D::DrawQuad(overrideQuadTransform, m_Texture, m_OverrideMaterial, 1.0f, glm::vec4(1.0f, 0.9f, 0.9f, 0.95f));

        glm::mat4 spriteOverrideTransform =
            glm::translate(glm::mat4(1.0f), glm::vec3(92.0f, 65.0f, 0.1f)) *
            glm::rotate(glm::mat4(1.0f), glm::radians(12.0f), glm::vec3(0.0f, 0.0f, 1.0f)) *
            glm::scale(glm::mat4(1.0f), glm::vec3(34.0f, 34.0f, 1.0f));
        axiom::Renderer2D::DrawSprite(spriteOverrideTransform, m_Sprite, glm::vec4(1.0f, 0.85f, 0.85f, 1.0f), m_OverrideMaterial);

        // Skinned
        glm::mat4 skinnedTransform =
            glm::translate(glm::mat4(1.0f), glm::vec3(132.0f, 46.0f, 0.0f)) *
            glm::scale(glm::mat4(1.0f), glm::vec3(70.0f, 42.0f, 1.0f));
        axiom::Renderer2D::DrawSkinned(skinnedTransform, m_SkinnedMesh, m_Pose, glm::vec4(1.0f));

        glm::mat4 skinnedDebugTransform =
            glm::translate(glm::mat4(1.0f), glm::vec3(132.0f, 12.0f, 0.1f)) *
            glm::scale(glm::mat4(1.0f), glm::vec3(70.0f, 42.0f, 1.0f));
        axiom::Renderer2D::DrawSkinned(skinnedDebugTransform, m_SkinnedMesh, m_Pose, glm::vec4(1.0f), m_SkinnedDebugMaterial);

        // Lines / Rects / Circles
        axiom::Renderer2D::DrawLine(glm::vec3(-145.0f, -5.0f, 0.0f), glm::vec3(-108.0f, -28.0f, 0.0f), glm::vec4(1.0f, 0.9f, 0.2f, 1.0f), 3.0f, axiom::LineCap::Round);
        axiom::Renderer2D::DrawRect(glm::vec3(-82.0f, -30.0f, 0.0f), glm::vec2(34.0f, 22.0f), glm::vec4(0.2f, 0.95f, 0.95f, 1.0f), 0.0f, 2.5f);

        glm::mat4 rectTransform =
            glm::translate(glm::mat4(1.0f), glm::vec3(-25.0f, -22.0f, 0.0f)) *
            glm::rotate(glm::mat4(1.0f), glm::radians(28.0f), glm::vec3(0.0f, 0.0f, 1.0f)) *
            glm::scale(glm::mat4(1.0f), glm::vec3(30.0f, 18.0f, 1.0f));
        axiom::Renderer2D::DrawRect(rectTransform, glm::vec4(1.0f, 0.55f, 0.15f, 1.0f), 2.0f);

        // LineStrip Test
        std::vector<glm::vec3> polyline = {
            { -40.0f, -60.0f, 0.0f },
            { -10.0f, -80.0f, 0.0f },
            {  20.0f, -60.0f, 0.0f },
            {  50.0f, -80.0f, 0.0f }
        };
        axiom::Renderer2D::DrawLineStrip(polyline, glm::vec4(0.9f, 0.7f, 0.2f, 1.0f), 4.0f, false, axiom::LineCap::Round, axiom::LineJoin::Round, 4.0f);

        // Batch-Stress-Test
        for (int i = 0; i < 40; ++i) {
            for (int j = 0; j < 25; ++j) {
                float x = -150.0f + i * 8.0f;
                float y = -120.0f + j * 8.0f;
                float z = (i + j) * 0.0005f;
                glm::vec4 c = glm::vec4(
                    0.3f + 0.7f * (i / 40.0f),
                    0.3f + 0.7f * (j / 25.0f),
                    0.5f,
                    0.8f
                );
                axiom::Renderer2D::DrawQuad(glm::vec2(x, y), glm::vec2(6.0f, 6.0f), c, z);
            }
        }
    }

    void OnShutdown() override {
        m_SkinnedDebugMaterial.reset();
        m_OverrideMaterial.reset();
        m_Texture.reset();
        m_SkinnedTexture.reset();
        axiom::Renderer2D::Shutdown();
    }

private:
    void BuildEcsScene() {
        auto& scene = GetScene();

        m_EcsColoredQuad = scene.CreateEntity("ECS Colored Quad");
        auto& coloredTransform = m_EcsColoredQuad.GetComponent<axiom::TransformComponent>();
        coloredTransform.Translation = glm::vec3(-68.0f, 65.0f, 0.1f);
        coloredTransform.Scale = glm::vec3(22.0f, 14.0f, 1.0f);
        coloredTransform.Rotation.z = glm::radians(22.0f);
        m_EcsColoredQuad.AddComponent<axiom::SpriteRendererComponent>(glm::vec4(0.35f, 0.65f, 1.0f, 0.9f));

        m_EcsTexturedSprite = scene.CreateEntity("ECS Textured Sprite");
        auto& texturedTransform = m_EcsTexturedSprite.GetComponent<axiom::TransformComponent>();
        texturedTransform.Translation = glm::vec3(45.0f, 65.0f, 0.0f);
        texturedTransform.Scale = glm::vec3(34.0f, 34.0f, 1.0f);
        m_EcsTexturedSprite.AddComponent<axiom::SpriteRendererComponent>(m_Sprite, glm::vec4(1.0f));

        m_EcsCircle = scene.CreateEntity("ECS Circle");
        auto& circleTransform = m_EcsCircle.GetComponent<axiom::TransformComponent>();
        circleTransform.Translation = glm::vec3(115.0f, -20.0f, 0.0f);
        circleTransform.Scale = glm::vec3(26.0f, 26.0f, 1.0f);
        m_EcsCircle.AddComponent<axiom::CircleRendererComponent>(glm::vec4(0.25f, 0.5f, 1.0f, 1.0f), 0.35f);
    }

    void ApplyDebugCamera() {
        float width = static_cast<float>(GetWidth());
        float height = static_cast<float>(GetHeigth());
        float halfWidth = width * 0.5f / m_CameraZoom;
        float halfHeight = height * 0.5f / m_CameraZoom;

        glm::mat4 proj = glm::ortho(-halfWidth, halfWidth, -halfHeight, halfHeight, -1.0f, 1.0f);
        glm::mat4 view = glm::translate(glm::mat4(1.0f), -m_CameraPosition);
        ViewProjection = proj * view;
    }

private:
    std::shared_ptr<axiom::Texture2D> m_Texture;
    std::shared_ptr<axiom::Texture2D> m_SkinnedTexture;
    std::shared_ptr<axiom::Material> m_OverrideMaterial;
    std::shared_ptr<axiom::Material> m_SkinnedDebugMaterial;
    axiom::Sprite m_Sprite;
    axiom::SkinnedMesh2D m_SkinnedMesh;
    axiom::SkeletonPose2D m_Pose;
    axiom::Entity m_EcsColoredQuad;
    axiom::Entity m_EcsTexturedSprite;
    axiom::Entity m_EcsCircle;
    glm::vec3 m_CameraPosition{0.0f, 0.0f, 0.0f};
    float m_CameraZoom = 1.0f;
    glm::mat4 ViewProjection{1.0f};
};

namespace axiom {

Application* CreateApplication() {
    return new Testbed();
}

}

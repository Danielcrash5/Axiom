// main.cpp
#include <axiom/core/Application.h>
#include <axiom/core/Layer.h>
#include <axiom/core/Logger.h>
#include <axiom/core/Time.h>
#include <axiom/input/KeyCodes.h>
#include <axiom/input/Input.h>
#include <axiom/assets/AssetManager.h>
#include <axiom/assets/TextureLoadInfo.h>
#include <axiom/renderer/Renderer.h>
#include <axiom/renderer/Renderer2D.h>
#include <axiom/renderer/Texture2D.h>
#include <axiom/renderer/Material.h>
#include <axiom/renderer/Shader.h>
#include <axiom/renderer/Sprite.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>
#include <vector>

namespace {
    std::shared_ptr<axiom::Texture2D> LoadGameTexture(const std::string& virtualPath) {
        auto info = axiom::TexturePresets::Sprite();
        return axiom::AssetManager::Get<axiom::Texture2D>(virtualPath, info);
    }

    std::shared_ptr<axiom::Material> CreateTexturedOverrideMaterial() {
        static const char* source = R"(#type vertex
#version 460 core

layout(location = 0) in vec4 a_Position;
layout(location = 1) in vec4 a_Color;
layout(location = 2) in vec2 a_TexCoord;
layout(location = 3) in float a_TexIndex;
layout(location = 4) in float a_TilingFactor;

uniform mat4 u_ViewProjection;
uniform mat4 u_Transform;

out vec4 v_Color;
out vec2 v_TexCoord;

void main()
{
    v_Color = a_Color;
    v_TexCoord = a_TexCoord;
    gl_Position = u_ViewProjection * u_Transform * a_Position;
}

#type fragment
#version 460 core

layout(location = 0) out vec4 FragColor;

in vec4 v_Color;
in vec2 v_TexCoord;

uniform sampler2D u_Texture;

void main()
{
    vec4 tex = texture(u_Texture, v_TexCoord);
    FragColor = vec4(tex.bgr, tex.a) * v_Color;
}
)";

        return std::make_shared<axiom::Material>(axiom::Shader::CreateFromMemory(source));
    }

    std::shared_ptr<axiom::Material> CreateSkinnedColorOverrideMaterial() {
        static const char* source = R"(#type vertex
#version 460 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Color;
layout(location = 2) in vec2 a_TexCoord;
layout(location = 3) in vec4 a_BoneIndices;
layout(location = 4) in vec4 a_BoneWeights;

uniform mat4 u_ViewProjection;
uniform mat4 u_Transform;
uniform mat4 u_BoneTransforms[64];

out vec4 v_Color;

void main()
{
    mat4 skin =
        u_BoneTransforms[int(a_BoneIndices.x)] * a_BoneWeights.x +
        u_BoneTransforms[int(a_BoneIndices.y)] * a_BoneWeights.y +
        u_BoneTransforms[int(a_BoneIndices.z)] * a_BoneWeights.z +
        u_BoneTransforms[int(a_BoneIndices.w)] * a_BoneWeights.w;

    vec4 localPosition = skin * vec4(a_Position, 1.0);
    v_Color = a_Color;
    gl_Position = u_ViewProjection * u_Transform * localPosition;
}

#type fragment
#version 460 core

layout(location = 0) out vec4 FragColor;
in vec4 v_Color;

void main()
{
    FragColor = v_Color;
}
)";

        return std::make_shared<axiom::Material>(axiom::Shader::CreateFromMemory(source));
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

        m_Texture = LoadGameTexture("game://textures/Purple/texture_01.png");
        m_SkinnedTexture = LoadGameTexture("game://textures/Orange/texture_01.png");
        m_Sprite = axiom::Sprite(m_Texture, {0.0f, 0.0f}, {1.0f, 1.0f});
        m_OverrideMaterial = CreateTexturedOverrideMaterial();
        m_SkinnedDebugMaterial = CreateSkinnedColorOverrideMaterial();

        m_SkinnedMesh = CreateRibbonSkinnedMesh(m_SkinnedTexture);

        m_Pose.BoneCount = 2;
        m_Pose.BoneTransforms[0] = glm::mat4(1.0f);
        m_Pose.BoneTransforms[1] = glm::mat4(1.0f);

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
        m_Pose.BoneTransforms[0] = glm::mat4(1.0f);
        m_Pose.BoneTransforms[1] =
            glm::translate(glm::mat4(1.0f), glm::vec3(pivotX, bendLift, 0.0f)) *
            glm::rotate(glm::mat4(1.0f), swing, glm::vec3(0.0f, 0.0f, 1.0f)) *
            glm::translate(glm::mat4(1.0f), glm::vec3(-pivotX, 0.0f, 0.0f));

        axiom::Renderer::BeginScene(ViewProjection, {});

        axiom::Renderer2D::BeginScene();

        // Z-Layer Test
        axiom::Renderer2D::DrawQuad(glm::vec2(-130.0f, 65.0f), glm::vec2(18.0f, 18.0f), glm::vec4(0.95f, 0.35f, 0.35f, 1.0f), -0.5f);
        axiom::Renderer2D::DrawQuad(glm::vec3(-100.0f, 65.0f, 0.2f), glm::vec2(18.0f, 24.0f), glm::vec4(0.35f, 0.95f, 0.35f, 0.85f));

        glm::mat4 coloredTransform =
            glm::translate(glm::mat4(1.0f), glm::vec3(-68.0f, 65.0f, 0.1f)) *
            glm::rotate(glm::mat4(1.0f), glm::radians(22.0f), glm::vec3(0.0f, 0.0f, 1.0f)) *
            glm::scale(glm::mat4(1.0f), glm::vec3(22.0f, 14.0f, 1.0f));
        axiom::Renderer2D::DrawQuad(coloredTransform, glm::vec4(0.35f, 0.65f, 1.0f, 0.9f));

        // Textured
        axiom::Renderer2D::DrawQuad(glm::vec2(-130.0f, 20.0f), glm::vec2(32.0f, 32.0f), m_Texture, 1.0f, glm::vec4(1.0f), -0.1f);
        axiom::Renderer2D::DrawQuad(glm::vec3(-86.0f, 20.0f, 0.1f), glm::vec2(36.0f, 24.0f), m_Texture, 1.0f, glm::vec4(1.0f, 1.0f, 1.0f, 0.85f));

        glm::mat4 texturedTransform =
            glm::translate(glm::mat4(1.0f), glm::vec3(-40.0f, 20.0f, 0.0f)) *
            glm::rotate(glm::mat4(1.0f), glm::radians(-18.0f), glm::vec3(0.0f, 0.0f, 1.0f)) *
            glm::scale(glm::mat4(1.0f), glm::vec3(34.0f, 34.0f, 1.0f));
        axiom::Renderer2D::DrawQuad(texturedTransform, m_Texture, 1.0f, glm::vec4(1.0f));

        glm::mat4 overrideQuadTransform =
            glm::translate(glm::mat4(1.0f), glm::vec3(5.0f, 20.0f, 0.2f)) *
            glm::scale(glm::mat4(1.0f), glm::vec3(34.0f, 34.0f, 1.0f));
        axiom::Renderer2D::DrawQuad(overrideQuadTransform, m_Texture, m_OverrideMaterial, 1.0f, glm::vec4(1.0f, 0.9f, 0.9f, 0.95f));

        // Sprites
        glm::mat4 spriteTransform =
            glm::translate(glm::mat4(1.0f), glm::vec3(45.0f, 65.0f, 0.0f)) *
            glm::scale(glm::mat4(1.0f), glm::vec3(34.0f, 34.0f, 1.0f));
        axiom::Renderer2D::DrawSprite(spriteTransform, m_Sprite, glm::vec4(1.0f));

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

        axiom::Renderer2D::DrawCircle(glm::vec2(35.0f, -20.0f), 13.0f, 0.0f, glm::vec4(0.95f, 0.2f, 0.2f, 1.0f));
        axiom::Renderer2D::DrawCircle(glm::vec3(72.0f, -20.0f, 0.1f), 13.0f, 0.2f, glm::vec4(0.2f, 0.95f, 0.2f, 1.0f));

        glm::mat4 circleTransform =
            glm::translate(glm::mat4(1.0f), glm::vec3(115.0f, -20.0f, 0.0f)) *
            glm::scale(glm::mat4(1.0f), glm::vec3(26.0f, 26.0f, 1.0f));
        axiom::Renderer2D::DrawCircle(circleTransform, 0.35f, glm::vec4(0.25f, 0.5f, 1.0f, 1.0f));

        // LineStrip Test
        std::vector<glm::vec3> polyline = {
            { -40.0f, -60.0f, 0.0f },
            { -10.0f, -80.0f, 0.0f },
            {  20.0f, -60.0f, 0.0f },
            {  50.0f, -80.0f, 0.0f }
        };
        axiom::Renderer2D::DrawLineStrip(polyline, glm::vec4(0.9f, 0.7f, 0.2f, 1.0f), 4.0f, false, axiom::LineCap::Round, axiom::LineJoin::Round, 4.0f);

        //// Batch-Stress-Test
        //for (int i = 0; i < 40; ++i) {
        //    for (int j = 0; j < 25; ++j) {
        //        float x = -150.0f + i * 8.0f;
        //        float y = -120.0f + j * 8.0f;
        //        float z = (i + j) * 0.0005f;
        //        glm::vec4 c = glm::vec4(
        //            0.3f + 0.7f * (i / 40.0f),
        //            0.3f + 0.7f * (j / 25.0f),
        //            0.5f,
        //            0.8f
        //        );
        //        axiom::Renderer2D::DrawQuad(glm::vec2(x, y), glm::vec2(6.0f, 6.0f), c, z);
        //    }
        //}

        axiom::Renderer2D::EndScene();
    }

    void OnShutdown() override {
        m_SkinnedDebugMaterial.reset();
        m_OverrideMaterial.reset();
        m_Texture.reset();
        m_SkinnedTexture.reset();
        axiom::Renderer2D::Shutdown();
    }

private:
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
    glm::vec3 m_CameraPosition{0.0f, 0.0f, 0.0f};
    float m_CameraZoom = 1.0f;
    glm::mat4 ViewProjection{1.0f};
};

int main() {
    Testbed game;
    game.Run();
    return 0;
}

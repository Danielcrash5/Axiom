#pragma once
#include <axiom/core/Layer.h>
#include <axiom/renderer/Renderer2D.h>
#include <axiom/renderer/Texture.h>
#include <axiom/core/Logger.h>

class TestScene : public axiom::Layer {
public:
    TestScene()
        : axiom::Layer("TestScene") {}

    void OnAttach() override {
        AXIOM_INFO("TestScene attached");
        // load a texture
        axiom::TextureSpecification spec;
        m_Texture = std::make_shared<axiom::Texture>(axiom::TextureType::Texture2D, spec);
        // Try loading an image from assets; fallback to allocated empty texture
        try {
            m_Texture->loadFromFile("assets/textures/Green/texture_01.png");
        } catch (...) {
            AXIOM_WARN("Failed to load assets/textures/Green/texture_01.png, using empty texture");
            m_Texture->allocate();
        }
    }

    void OnRender(double alpha) override {
        // draw a textured quad
        axiom::Renderer2D::DrawQuad({ 100.0f, 100.0f, 0.0f }, { 200.0f, 150.0f }, m_Texture);

        // draw a circle
        axiom::Renderer2D::DrawCircle({ 400.0f, 300.0f, 0.0f }, 50.0f, glm::vec4(1.0f, 1.0f, 0.0f, 1.0f));

        // draw a small colored quad (blue)
        axiom::Renderer2D::DrawQuad({ 120.0f, 60.0f, 0.0f }, { 60.0f, 20.0f }, glm::vec4(0.2f, 0.6f, 1.0f, 1.0f), 1);

        // draw a red line strip
        std::vector<glm::vec3> pts = { {50.0f,50.0f,0.0f}, {150.0f,200.0f,0.0f}, {300.0f,180.0f,0.0f}, {420.0f,350.0f,0.0f} };
        axiom::Renderer2D::DrawLineStrip(pts, glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), 0.02f, 1);
    }

private:
    std::shared_ptr<axiom::Texture> m_Texture;
};

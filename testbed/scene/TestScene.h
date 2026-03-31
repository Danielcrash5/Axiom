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
            m_Texture->loadFromFile("assets/textures/Dark/texture_01.png");
        } catch (...) {
            AXIOM_WARN("Failed to load assets/textures/Dark/texture_01.png, using empty texture");
            m_Texture->allocate();
        }
    }

    void OnRender(double alpha) override {
        // draw a few quads and a line strip
        // textured quad
        axiom::Renderer2D::DrawQuad({ 100.0f, 100.0f, 0.0f }, { 200.0f, 150.0f }, m_Texture);
        axiom::Renderer2D::DrawCircle({ 400.0f, 300.0f, 0.0f }, 50.0f);

        // draw a small colored quad as a text placeholder ("Textures")
        // In absence of a text renderer, this emulates a label
        glm::mat4 transform = glm::translate(glm::mat4(1.0f), glm::vec3(120, 60, 0))
            * glm::scale(glm::mat4(1.0f), glm::vec3(60.0f, 20.0f, 1.0f));
        auto mat = std::make_shared<axiom::Material>(nullptr);
        mat->setVec4("u_Color", glm::vec4(0.2f, 0.6f, 1.0f, 1.0f));
        axiom::Renderer2D::DrawQuad(transform, mat);
        std::vector<glm::vec3> pts = { {50,50,0}, {150,200,0}, {300,180,0}, {420,350,0} };
        axiom::Renderer2D::DrawLineStrip(pts);
    }

private:
    std::shared_ptr<axiom::Texture> m_Texture;
};

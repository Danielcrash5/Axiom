#include <axiom/core/Application.h>
#include <axiom/core/Layer.h>
#include <axiom/input/KeyCodes.h>
#include <axiom/core/Logger.h>
#include <axiom/renderer/Renderer2D.h>
#include <axiom/renderer/Texture.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

class InputTestLayer : public axiom::Layer {
public:
	InputTestLayer() :
		Layer("InputTestLayer") {}
	void OnAttach() override {
		AXIOM_INFO("InputTestLayer attached!");
	}
	void OnFixedUpdate(double dt) override {
		auto input = GetMainInput();
		if (input.IsKeyPressed(axiom::Key::Space)) {
			AXIOM_INFO("Space key is pressed!");
		}
	}
	void OnDetach() override {
		AXIOM_INFO("InputTestLayer detached!");
	}
};

class Testbed : public axiom::Application {
public:
	Testbed() :
		Application("testbed") {
		// Nie etwas hier hinein schreiben
	}
protected:
	void OnInit() override {
		PushLayer(std::make_unique<InputTestLayer>());
		axiom::Renderer2D::Init();
		projection = glm::ortho(
			0.0f, 1280.0f,   // left, right
			0.0f, 720.0f,    // bottom, top
			-1.0f, 1.0f      // near, far
		);
		texture1 = std::make_shared<axiom::Texture>("assets/textures/Dark/texture_01.png");
		texture2 = std::make_shared<axiom::Texture>("assets/textures/Green/texture_03.png");
	}

	void OnRender() override {
		axiom::Renderer2D::Begin(projection);
		axiom::Renderer2D::DrawQuad(position, glm::vec2(200.0f, 200.0f), glm::vec4(1, 1, 1, 1));
		axiom::Renderer2D::DrawQuad({360.0f, 0.1f, 100.0f}, {500.0f, 500.0f}, texture1);
		axiom::Renderer2D::DrawQuad({360.0f, 0.05f, 100.0f}, {500.0f, 500.0f}, 45.0f, texture2);
		axiom::Renderer2D::End();
	}

	void OnUpdate(double dt) override {
		auto input = GetMainInput();
		if (input.IsKeyPressed(axiom::Key::D)) {
			position.x += 500 * dt;
		}
		if (input.IsKeyPressed(axiom::Key::A)) {
			position.x -= 500 * dt;
		}
		if (input.IsKeyPressed(axiom::Key::W)) {
			position.y += 500 * dt;
		}
		if (input.IsKeyPressed(axiom::Key::S)) {
			position.y -= 500 * dt;
		}
	}

	void OnShutdown() override {
		axiom::Renderer2D::Shutdown();
	}

	glm::mat4 projection;
	glm::vec3 position = { 0.0f, 0.0f, 0.2f };

	std::shared_ptr<axiom::Texture> texture1;
	std::shared_ptr<axiom::Texture> texture2;
};

int main() {
	Testbed game;
	game.Run();
	return 0;
}
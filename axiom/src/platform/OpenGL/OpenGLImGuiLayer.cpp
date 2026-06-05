#include "axiom/platform/OpenGL/OpenGLImGuiLayer.h"
#include "axiom/assets/VFS.h"
#include "axiom/core/Logger.h"
#include <cstring>

#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_opengl3.h>

namespace axiom {
	void ApplyCustomAxiomStyle() {
		ImGuiStyle& style = ImGui::GetStyle();
		ImVec4* colors = style.Colors;

		style.WindowRounding = 6.0f;
		style.FrameRounding = 4.0f;
		style.PopupRounding = 4.0f;
		style.GrabRounding = 4.0f;
		style.TabRounding = 4.0f;
		style.ScrollbarRounding = 9.0f;

		colors[ImGuiCol_Text] = ImVec4(0.95f, 0.96f, 0.98f, 1.00f);
		colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
		colors[ImGuiCol_WindowBg] = ImVec4(0.12f, 0.12f, 0.14f, 1.00f);
		colors[ImGuiCol_ChildBg] = ImVec4(0.15f, 0.15f, 0.17f, 1.00f);
		colors[ImGuiCol_PopupBg] = ImVec4(0.12f, 0.12f, 0.14f, 1.00f);
		colors[ImGuiCol_Border] = ImVec4(0.25f, 0.25f, 0.27f, 1.00f);
		colors[ImGuiCol_FrameBg] = ImVec4(0.20f, 0.20f, 0.22f, 1.00f);
		colors[ImGuiCol_FrameBgHovered] = ImVec4(0.25f, 0.25f, 0.27f, 1.00f);
		colors[ImGuiCol_FrameBgActive] = ImVec4(0.30f, 0.30f, 0.33f, 1.00f);

		colors[ImGuiCol_TitleBg] = ImVec4(0.08f, 0.08f, 0.09f, 1.00f);
		colors[ImGuiCol_TitleBgActive] = ImVec4(0.12f, 0.12f, 0.14f, 1.00f);
		colors[ImGuiCol_Header] = ImVec4(0.25f, 0.25f, 0.27f, 1.00f);
		colors[ImGuiCol_HeaderHovered] = ImVec4(0.35f, 0.35f, 0.40f, 1.00f);
		colors[ImGuiCol_HeaderActive] = ImVec4(0.40f, 0.40f, 0.45f, 1.00f);

		colors[ImGuiCol_Button] = ImVec4(0.30f, 0.40f, 0.70f, 1.00f);
		colors[ImGuiCol_ButtonHovered] = ImVec4(0.40f, 0.50f, 0.80f, 1.00f);
		colors[ImGuiCol_ButtonActive] = ImVec4(0.20f, 0.30f, 0.60f, 1.00f);

		colors[ImGuiCol_DockingPreview] = ImVec4(0.30f, 0.40f, 0.70f, 0.70f);
		colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.08f, 0.08f, 0.08f, 1.00f);
	}

	static ImFont* AddFontFromVFS(
		ImFontAtlas* atlas,
		const char* vfsPath,
		float sizePixels,
		const ImFontConfig& fontConfig
	) {
		std::vector<uint8_t> fileData;
		const bool readOk = VFS::ReadFile(vfsPath, fileData);
		AXIOM_INFO("VFS font load path='{}' ok={} size={}", vfsPath, readOk, fileData.size());

		if (!readOk || fileData.empty()) {
			return nullptr;
		}

		void* atlasOwnedData = IM_ALLOC(fileData.size());
		if (atlasOwnedData == nullptr) {
			AXIOM_ERROR("Failed to allocate {} bytes for ImGui font '{}'.", fileData.size(), vfsPath);
			return nullptr;
		}

		std::memcpy(atlasOwnedData, fileData.data(), fileData.size());

		ImFont* font = atlas->AddFontFromMemoryTTF(
			atlasOwnedData,
			static_cast<int>(fileData.size()),
			sizePixels,
			&fontConfig
		);

		if (font == nullptr) {
			IM_FREE(atlasOwnedData);
			AXIOM_ERROR("ImGui rejected font '{}'.", vfsPath);
		}

		return font;
	}

	OpenGLImGuiLayer::OpenGLImGuiLayer(
		std::unique_ptr<Window>& window) {
		m_SDLWindow =
			static_cast<SDL_Window*>(window->GetNativeHandle());

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();

		ImGuiIO& io = ImGui::GetIO();

		ApplyCustomAxiomStyle();

		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		io.ConfigFlags |= ImGuiConfigFlags_DpiEnableScaleViewports;
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
		

		/*
		// =========================================================================
	   // STANDARD-SCHRIFTARTEN AUS DEM VFS IN DEN FONT-ATLAS LADEN
	   // =========================================================================
		ImFontConfig fontConfig{};
		fontConfig.PixelSnapH = true; // Verhindert verschwommene Kanten

		// 1. INTER REGULAR (Wird automatisch der Standard-Font für die gesamte UI)
		// m_DefaultFont = AddFontFromVFS(io.Fonts,			"engine://fonts/inter/static/Inter_18pt-Black.ttf",			18.0f,			fontConfig			);
		if (m_DefaultFont == nullptr) {
			AXIOM_ERROR("Failed to load Inter font from VFS. ImGui will use default font.");
		}

		// 2. JETBRAINS MONO REGULAR (Für Konsole, Logg-Ausgaben oder Text-Editoren)
		m_MonospaceFont = AddFontFromVFS(
			io.Fonts,
			"engine://fonts/JetBrains-Mono/static/JetBrainsMono-Regular.ttf",
			15.0f,
			fontConfig
		);
		if (m_MonospaceFont == nullptr) {
			AXIOM_ERROR("Failed to load JetBrains Mono font from VFS. ImGui will use default font for monospace text.");
		}

		if (m_DefaultFont == nullptr) {
			m_DefaultFont = io.Fonts->AddFontDefault();
			AXIOM_INFO("Using Dear ImGui default font as UI fallback.");
		}

		if (m_MonospaceFont == nullptr) {
			m_MonospaceFont = m_DefaultFont;
			AXIOM_INFO("Using UI font as monospace fallback.");
		}

		io.FontDefault = m_DefaultFont;
		io.Fonts->Build();
		AXIOM_INFO("Successfully initialized ImGui fonts.");*/

		SDL_GLContext glContext =
			SDL_GL_GetCurrentContext();

		ImGui_ImplSDL3_InitForOpenGL(
			m_SDLWindow,
			glContext);

		ImGui_ImplOpenGL3_Init("#version 330 core");
	}

	OpenGLImGuiLayer::~OpenGLImGuiLayer() {
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplSDL3_Shutdown();
		ImGui::DestroyContext();
	}

	void OpenGLImGuiLayer::Begin() {
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL3_NewFrame();
		ImGui::NewFrame();

		RenderDockspace();
	}

	void OpenGLImGuiLayer::End() {
		ImGui::Render();

		ImGui_ImplOpenGL3_RenderDrawData(
			ImGui::GetDrawData());

		ImGuiIO& io = ImGui::GetIO();

		if (io.ConfigFlags &
			ImGuiConfigFlags_ViewportsEnable) {
			SDL_Window* backupWindow =
				SDL_GL_GetCurrentWindow();

			SDL_GLContext backupContext =
				SDL_GL_GetCurrentContext();

			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();

			SDL_GL_MakeCurrent(
				backupWindow,
				backupContext);
		}
	}

	void OpenGLImGuiLayer::RenderDockspace() {
		if (m_DockspaceType == DockspaceType::None) {
			return; // Nichts tun
		}

		// Viewport-Daten holen, um das Dockspace über das gesamte Fenster zu spannen
		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->WorkPos);
		ImGui::SetNextWindowSize(viewport->WorkSize);
		ImGui::SetNextWindowViewport(viewport->ID);

		// Standard-Flags für das umgebende Trägerfenster
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus |
			ImGuiWindowFlags_NoNavFocus;

		// Wenn "Passthrough" aktiv ist, machen wir das Trägerfenster unsichtbar
		if (m_DockspaceType == DockspaceType::Passthrough) {
			window_flags |= ImGuiWindowFlags_NoBackground;
		}

		// Trägerfenster pushen (ohne Padding, damit es bündig abschließt)
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("AxiomEngineDockspace_Host", nullptr, window_flags);
		ImGui::PopStyleVar();

		// Das eigentliche Dockspace-ID-System aktivieren
		ImGuiID dockspace_id = ImGui::GetID("AxiomEngineMainDockspace");

		// Passthrough-spezifische Flags für das Docking-System selbst
		ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;
		if (m_DockspaceType == DockspaceType::Passthrough) {
			dockspace_flags |= ImGuiDockNodeFlags_PassthruCentralNode;
		}

		ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		ImGui::End();
	}

}
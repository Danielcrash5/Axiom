#include "axiom/ImGui/VulkanImGuiLayer.h"

#include "axiom/assets/VFS.h"
#include "axiom/core/Logger.h"
#include "axiom/platform/Window.h"
#include "axiom/renderer/Renderer.h"
#include "axiom/renderer/rendergraph/RenderPass.h"
#include "axiom/renderer/vulkan/VulkanBackend.h"
#include "axiom/renderer/vulkan/VulkanCommandlist.h"

#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_vulkan.h>

#include <limits>
#include <optional>

namespace axiom {

    namespace {

        namespace rg = renderer::rendergraph;
        namespace rhi = renderer::rhi;

        ImVec4 ThemeColor(uint32_t rgb, float alpha = 1.0f) {
            return ImVec4(
                static_cast<float>((rgb >> 16) & 0xFF) / 255.0f,
                static_cast<float>((rgb >> 8) & 0xFF) / 255.0f,
                static_cast<float>(rgb & 0xFF) / 255.0f, alpha);
        }

        void ApplyAxiomTheme() {
            ImGui::StyleColorsDark();

            ImGuiStyle &style = ImGui::GetStyle();
            style.WindowPadding = ImVec2(10.0f, 8.0f);
            style.FramePadding = ImVec2(7.0f, 4.0f);
            style.CellPadding = ImVec2(7.0f, 5.0f);
            style.ItemSpacing = ImVec2(8.0f, 6.0f);
            style.ItemInnerSpacing = ImVec2(6.0f, 4.0f);
            style.IndentSpacing = 16.0f;
            style.ScrollbarSize = 12.0f;
            style.GrabMinSize = 8.0f;

            style.WindowRounding = 5.0f;
            style.ChildRounding = 4.0f;
            style.FrameRounding = 3.0f;
            style.PopupRounding = 4.0f;
            style.ScrollbarRounding = 8.0f;
            style.GrabRounding = 3.0f;
            style.TabRounding = 4.0f;

            style.WindowBorderSize = 1.0f;
            style.ChildBorderSize = 1.0f;
            style.PopupBorderSize = 1.0f;
            style.FrameBorderSize = 0.0f;
            style.TabBorderSize = 0.0f;
            style.WindowTitleAlign = ImVec2(0.0f, 0.5f);

            const auto set = [&style](ImGuiCol color, uint32_t rgb,
                                      float alpha = 1.0f) {
                style.Colors[color] = ThemeColor(rgb, alpha);
            };

            set(ImGuiCol_Text, 0xE2E7EF);
            set(ImGuiCol_TextDisabled, 0x858F9E);
            set(ImGuiCol_WindowBg, 0x20242B);
            set(ImGuiCol_ChildBg, 0x252A32);
            set(ImGuiCol_PopupBg, 0x2B3039, 0.98f);
            set(ImGuiCol_Border, 0x3B424D);
            set(ImGuiCol_BorderShadow, 0x101216, 0.0f);
            set(ImGuiCol_FrameBg, 0x2D333D);
            set(ImGuiCol_FrameBgHovered, 0x3A4049);
            set(ImGuiCol_FrameBgActive, 0x454C56);
            set(ImGuiCol_TitleBg, 0x1B1F25);
            set(ImGuiCol_TitleBgActive, 0x252B34);
            set(ImGuiCol_TitleBgCollapsed, 0x1B1F25, 0.75f);
            set(ImGuiCol_MenuBarBg, 0x252A32);
            set(ImGuiCol_ScrollbarBg, 0x191C21, 0.55f);
            set(ImGuiCol_ScrollbarGrab, 0x414A57);
            set(ImGuiCol_ScrollbarGrabHovered, 0x626B78);
            set(ImGuiCol_ScrollbarGrabActive, 0x7B8592);
            set(ImGuiCol_CheckMark, 0xB7C0CC);
            set(ImGuiCol_SliderGrab, 0x8B96A4);
            set(ImGuiCol_SliderGrabActive, 0xB7C0CC);
            set(ImGuiCol_Button, 0x343B46);
            set(ImGuiCol_ButtonHovered, 0x424953);
            set(ImGuiCol_ButtonActive, 0x505863);
            set(ImGuiCol_Header, 0x363C45);
            set(ImGuiCol_HeaderHovered, 0x424953);
            set(ImGuiCol_HeaderActive, 0x505863);
            set(ImGuiCol_Separator, 0x3B424D);
            set(ImGuiCol_SeparatorHovered, 0x77818E);
            set(ImGuiCol_SeparatorActive, 0xA1ABB8);
            set(ImGuiCol_ResizeGrip, 0x8B96A4, 0.22f);
            set(ImGuiCol_ResizeGripHovered, 0x8B96A4, 0.65f);
            set(ImGuiCol_ResizeGripActive, 0xB7C0CC, 0.90f);
            set(ImGuiCol_Tab, 0x292F38);
            set(ImGuiCol_TabHovered, 0x424953);
            set(ImGuiCol_TabSelected, 0x343B46);
            set(ImGuiCol_TabSelectedOverline, 0xA1ABB8);
            set(ImGuiCol_TabDimmed, 0x22272E);
            set(ImGuiCol_TabDimmedSelected, 0x2D333D);
            set(ImGuiCol_TabDimmedSelectedOverline, 0x727D8A);
            set(ImGuiCol_DockingPreview, 0x8B96A4, 0.35f);
            set(ImGuiCol_DockingEmptyBg, 0x191C21);
            set(ImGuiCol_PlotLines, 0xA1ABB8);
            set(ImGuiCol_PlotLinesHovered, 0xD0D6DE);
            set(ImGuiCol_PlotHistogram, 0xA9A08E);
            set(ImGuiCol_PlotHistogramHovered, 0xC5BBA8);
            set(ImGuiCol_TableHeaderBg, 0x2D333D);
            set(ImGuiCol_TableBorderStrong, 0x414955);
            set(ImGuiCol_TableBorderLight, 0x343A44);
            set(ImGuiCol_TableRowBg, 0xFFFFFF, 0.0f);
            set(ImGuiCol_TableRowBgAlt, 0xFFFFFF, 0.035f);
            set(ImGuiCol_TextSelectedBg, 0x8B96A4, 0.35f);
            set(ImGuiCol_DragDropTarget, 0xD0D6DE);
            set(ImGuiCol_NavHighlight, 0xB7C0CC);
            set(ImGuiCol_NavWindowingHighlight, 0xB7C0CC, 0.70f);
            set(ImGuiCol_NavWindowingDimBg, 0x101318, 0.20f);
            set(ImGuiCol_ModalWindowDimBg, 0x101318, 0.55f);
        }

        ImFont *LoadFont(ImFontAtlas &atlas, const char *path, float sizePixels,
                         std::vector<uint8_t> &fontData, bool required) {
            if (!VFS::ReadFile(path, fontData) || fontData.empty()) {
                if (required) {
                    AXIOM_ERROR("ImGui-Font konnte nicht aus VFS geladen werden: {}", path);
                } else {
                    AXIOM_WARN("Optionaler ImGui-Font konnte nicht aus VFS geladen werden: {}", path);
                }
                return nullptr;
            }

            if (fontData.size() > static_cast<size_t>(std::numeric_limits<int>::max())) {
                AXIOM_ERROR("ImGui-Font ist zu gross: {}", path);
                fontData.clear();
                return nullptr;
            }

            ImFontConfig config;
            config.FontDataOwnedByAtlas = false;
            ImFont *font = atlas.AddFontFromMemoryTTF(
                fontData.data(), static_cast<int>(fontData.size()), sizePixels,
                &config, atlas.GetGlyphRangesDefault());
            if (!font) {
                AXIOM_ERROR("ImGui konnte Fontdatei nicht verarbeiten: {}", path);
                fontData.clear();
            }
            return font;
        }

        void CheckVkResult(VkResult result) {
            if (result != VK_SUCCESS)
                AXIOM_ERROR("ImGui Vulkan-Backend: VkResult {}",
                            static_cast<int>(result));
        }

        // Schreibt die von ImGui::Render() erzeugten DrawData ins
        // Swapchain-Image des Hauptfensters. Clear oder Load entscheidet
        // der Graph: ist ImGui der erste Schreiber aufs Present-Target
        // (kein Szenen-Pass davor), cleart es per loadOp; laeuft davor ein
        // Pass, der das Bild schon beschrieben hat, LAEDT es und
        // ueberzeichnet nur.
        class ImGuiPass final : public rg::RenderPass {
          public:
            explicit ImGuiPass(rhi::SurfaceHandle surface)
                : m_Surface(surface) {}

            void setup(rg::RenderGraphBuilder &builder) override {
                m_Target = {}; // pro compile() neu bestimmen

                const renderer::View *view = builder.currentView();
                const rg::ResourceHandle present = builder.presentTargetResource();
                if (!view || !present.valid() ||
                    !(view->target.surface == m_Surface))
                    return; // andere View / kein Swapchain-Ziel: nichts tun

                m_Target = builder.write(present, rhi::TextureLayout::ColorAttachment);
            }

            void execute(rg::RenderContext &ctx, rhi::CommandList &cmd) override {
                if (!m_Target.valid())
                    return;

                std::optional<rhi::ClearColor> clear;
                if (ctx.isFirstWrite(m_Target))
                    clear = rhi::ClearColor{0.05f, 0.05f, 0.08f, 1.0f};

                cmd.beginRendering(ctx.resolveTexture(m_Target), std::nullopt,
                                   clear);

                if (ImGui::GetCurrentContext()) {
                    if (ImDrawData *drawData = ImGui::GetDrawData()) {
                        auto &vkCmd =
                            static_cast<rhi::vulkan::VulkanCommandList &>(cmd);
                        ImGui_ImplVulkan_RenderDrawData(drawData,
                                                        vkCmd.nativeHandle());
                    }
                }

                cmd.endRendering();
            }

            [[nodiscard]] const char *name() const override { return "ImGuiPass"; }

          private:
            rhi::SurfaceHandle m_Surface;
            rg::ResourceHandle m_Target;
        };

    } // namespace

    std::unique_ptr<VulkanImGuiLayer>
    VulkanImGuiLayer::Create(Window &window, renderer::Renderer &renderer) {
        auto *vulkan =
            dynamic_cast<rhi::vulkan::VulkanBackend *>(&renderer.backend());
        if (!vulkan) {
            AXIOM_ERROR("VulkanImGuiLayer: Renderer nutzt kein Vulkan-Backend");
            return nullptr;
        }

        // Swapchain (und damit Format) muss VOR dem Backend-Init feststehen:
        // die ImGui-Pipeline wird gegen genau dieses Attachment-Format gebaut.
        auto format = renderer.surfaceFormat(renderer.mainSurface(),
                                             window.GetWidth(), window.GetHeight());
        if (!format) {
            AXIOM_ERROR("VulkanImGuiLayer: Swapchain-Format nicht ermittelbar");
            return nullptr;
        }

        VkFormat vkFormat = VK_FORMAT_UNDEFINED;
        switch (*format) {
        case rhi::TextureFormat::BGRA8Unorm:
            vkFormat = VK_FORMAT_B8G8R8A8_UNORM;
            break;
        case rhi::TextureFormat::RGBA8Unorm:
            vkFormat = VK_FORMAT_R8G8B8A8_UNORM;
            break;
        default:
            AXIOM_ERROR("VulkanImGuiLayer: nicht unterstuetztes Swapchain-Format");
            return nullptr;
        }

        const auto native = vulkan->nativeContext();

        std::unique_ptr<VulkanImGuiLayer> layer(new VulkanImGuiLayer());
        layer->m_Device = native.device;
        layer->m_ColorFormat = vkFormat;

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        layer->m_ContextCreated = true;

        ImGuiIO &io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        // Jedes rausgezogene Fenster bekommt eine eigene, vom Backend
        // selbst verwaltete Swapchain - komplett am RenderGraph vorbei.
        // ImGui_ImplSDL3/_Vulkan erkennen das Flag selbst und richten
        // Platform_CreateVkSurface etc. automatisch ein (siehe
        // ImGui_ImplSDL3_InitMultiViewportSupport / _InitMultiViewportSupport
        // in imgui_impl_vulkan.cpp) - hier ist nichts weiter noetig als das
        // Flag VOR den beiden Init-Aufrufen unten zu setzen.
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
        ApplyAxiomTheme();

        ImFont *interBold = LoadFont(
            *io.Fonts, "engine://fonts/inter/static/Inter_18pt-Bold.ttf",
            17.0f, layer->m_interBoldFontData, false);
        ImFont *interRegular = LoadFont(
            *io.Fonts, "engine://fonts/inter/static/Inter_18pt-Regular.ttf",
            17.0f, layer->m_interRegularFontData, true);
        LoadFont(*io.Fonts,
                 "engine://fonts/JetBrains-Mono/static/JetBrainsMono-Regular.ttf",
                 16.0f, layer->m_jetBrainsMonoFontData, false);
        if (!interRegular) {
            return nullptr;
        }
        io.FontDefault = interRegular;
        if (!interBold) {
            AXIOM_WARN("ImGui nutzt Inter Regular als Fallback fuer fette UI-Texte");
        }

        if (!ImGui_ImplSDL3_InitForVulkan(window.GetNativeHandle())) {
            AXIOM_ERROR("ImGui_ImplSDL3_InitForVulkan fehlgeschlagen");
            return nullptr; // Destruktor raeumt den Context auf
        }
        layer->m_SDLBackendReady = true;

        ImGui_ImplVulkan_InitInfo info{};
        info.ApiVersion = VK_API_VERSION_1_3;
        info.Instance = native.instance;
        info.PhysicalDevice = native.physicalDevice;
        info.Device = native.device;
        info.QueueFamily = native.graphicsQueueFamily;
        info.Queue = native.graphicsQueue;
        info.DescriptorPoolSize = 64; // Backend legt den Pool selbst an
        info.MinImageCount = 2;
        // Der Backend-Ring fuer Vertex-/Index-Buffer hat ImageCount Slots; der
        // Renderer wartet aktuell pro Frame auf die GPU (1 Frame in flight),
        // 2 reicht also. Bei echten Frames-in-flight: ImageCount >= diese Zahl.
        info.ImageCount = 2;
        info.UseDynamicRendering = true;
        info.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
        auto &rendering = info.PipelineInfoMain.PipelineRenderingCreateInfo;
        rendering = {};
        rendering.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR;
        rendering.colorAttachmentCount = 1;
        rendering.pColorAttachmentFormats = &layer->m_ColorFormat;
        info.CheckVkResultFn = CheckVkResult;

        if (!ImGui_ImplVulkan_Init(&info)) {
            AXIOM_ERROR("ImGui_ImplVulkan_Init fehlgeschlagen");
            return nullptr;
        }
        layer->m_VulkanBackendReady = true;

        renderer.addPass(
            std::make_unique<ImGuiPass>(renderer.mainSurface()));

        return layer;
    }

    VulkanImGuiLayer::~VulkanImGuiLayer() {
        // GPU muss fertig sein, bevor Pipelines/Buffer/Font-Textur fallen.
        if (m_Device != VK_NULL_HANDLE)
            vkDeviceWaitIdle(m_Device);
        if (m_VulkanBackendReady)
            ImGui_ImplVulkan_Shutdown();
        if (m_SDLBackendReady)
            ImGui_ImplSDL3_Shutdown();
        if (m_ContextCreated)
            ImGui::DestroyContext();
    }

    void VulkanImGuiLayer::Begin() {
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        switch (m_DockspaceType) {
        case DockspaceType::Fullscreen:
            ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport());
            break;
        case DockspaceType::Passthrough:
            ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(),
                                         ImGuiDockNodeFlags_PassthruCentralNode);
            break;
        case DockspaceType::None:
            break;
        }
    }

    void VulkanImGuiLayer::End() { ImGui::Render(); }

    void VulkanImGuiLayer::RenderAdditionalViewports() {
        ImGuiIO &io = ImGui::GetIO();
        if (!(io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable))
            return;

        // Zeichnet + praesentiert jedes Sekundaerfenster ueber dessen eigene,
        // vom Vulkan-Backend selbst angelegte Swapchain. Muss NACH dem
        // Praesentieren des Hauptfensters laufen (siehe Application::MainLoop) -
        // parallel dazu waere ein zweites vkQueuePresentKHR auf derselben
        // Queue ohne zusaetzliche Synchronisation.
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }

    bool VulkanImGuiLayer::ProcessEvent(const SDL_Event &event) {
        ImGui_ImplSDL3_ProcessEvent(&event);

        const ImGuiIO &io = ImGui::GetIO();
        switch (event.type) {
        case SDL_EVENT_MOUSE_MOTION:
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
        case SDL_EVENT_MOUSE_WHEEL:
            return io.WantCaptureMouse;
        case SDL_EVENT_KEY_DOWN:
            return io.WantCaptureKeyboard;
        case SDL_EVENT_TEXT_INPUT:
            return io.WantTextInput;
        default:
            // *_UP-Events immer durchreichen (sonst haengende Tasten/Buttons,
            // wenn die Taste vor dem Fokuswechsel gedrueckt wurde), ebenso
            // Quit-/Window-Events.
            return false;
        }
    }

} // namespace axiom

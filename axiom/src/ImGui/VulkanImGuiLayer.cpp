#include "axiom/ImGui/VulkanImGuiLayer.h"

#include "axiom/core/Logger.h"
#include "axiom/platform/Window.h"
#include "axiom/renderer/Renderer.h"
#include "axiom/renderer/rendergraph/RenderPass.h"
#include "axiom/renderer/vulkan/VulkanBackend.h"
#include "axiom/renderer/vulkan/VulkanCommandlist.h"

#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_vulkan.h>

#include <optional>

namespace axiom {

    namespace {

        namespace rg = renderer::rendergraph;
        namespace rhi = renderer::rhi;

        void CheckVkResult(VkResult result) {
            if (result != VK_SUCCESS)
                AXIOM_ERROR("ImGui Vulkan-Backend: VkResult {}",
                            static_cast<int>(result));
        }

        // Schreibt die von ImGui::Render() erzeugten DrawData ins
        // Swapchain-Image des Hauptfensters. Clear passiert per loadOp -
        // dieser Pass ersetzt damit den ClearScreenPass, solange ImGui
        // der einzige Schreiber aufs Present-Target ist.
        class ImGuiPass final : public rg::RenderPass {
          public:
            ImGuiPass(rhi::SurfaceHandle surface, rhi::TextureFormat format)
                : m_Surface(surface), m_Format(format) {}

            void setup(rg::RenderGraphBuilder &builder) override {
                m_Target = {}; // pro compile() neu bestimmen

                const renderer::View *view = builder.currentView();
                const rhi::TextureHandle present = builder.presentTarget();
                if (!view || !present.valid() ||
                    !(view->target.surface == m_Surface))
                    return; // andere View / kein Swapchain-Ziel: nichts tun

                rg::TextureResourceDesc desc{
                    .width = view->viewport.width,
                    .height = view->viewport.height,
                    .format = m_Format,
                    // RenderTarget -> Graph legt ColorAttachment-Layout an.
                    .usage = rhi::TextureUsage::RenderTarget,
                    .debugName = "ImGuiPresentTarget",
                };
                m_Target = builder.write(builder.importTexture(present, desc));
            }

            void execute(rg::RenderContext &ctx, rhi::CommandList &cmd) override {
                if (!m_Target.valid())
                    return;

                cmd.beginRendering(ctx.resolveTexture(m_Target), std::nullopt,
                                   rhi::ClearColor{0.05f, 0.05f, 0.08f, 1.0f});

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
            rhi::TextureFormat m_Format;
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
        ImGui::StyleColorsDark();

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
            std::make_unique<ImGuiPass>(renderer.mainSurface(), *format));

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

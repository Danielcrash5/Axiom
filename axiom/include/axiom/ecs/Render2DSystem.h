#pragma once

#include "ISystem.h"
#include <axiom/renderer/View.h>

namespace axiom {

    namespace renderer {
        class Renderer;
    }

    class Render2DSystem : public ISystem {
      public:
        Render2DSystem(renderer::Renderer &renderer, renderer::ViewID viewId,
                       uint32_t viewportWidth = 1280,
                       uint32_t viewportHeight = 720);

        void SetViewport(uint32_t width, uint32_t height);
        void OnViewportResize(uint32_t width, uint32_t height) override;
        void Render(Scene &scene, double alpha) override;

      private:
        renderer::Renderer *m_Renderer = nullptr;
        renderer::ViewID m_ViewId = renderer::kInvalidViewID;
        uint32_t m_ViewportWidth = 1280;
        uint32_t m_ViewportHeight = 720;
    };

} // namespace axiom

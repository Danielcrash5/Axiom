#pragma once

#include "ISystem.h"

namespace axiom {

    class Render2DSystem : public ISystem {
      public:
        Render2DSystem(uint32_t viewportWidth = 1280,
                       uint32_t viewportHeight = 720);

        void SetViewport(uint32_t width, uint32_t height);
        void OnViewportResize(uint32_t width, uint32_t height) override;
        void BeginRenderFrame() override;
        void Render(Scene &scene, double alpha) override;

      private:
        uint32_t m_ViewportWidth = 1280;
        uint32_t m_ViewportHeight = 720;
        bool m_HasRenderedFrame = false;
    };

} // namespace axiom

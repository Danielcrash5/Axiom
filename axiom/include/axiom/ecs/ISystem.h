#pragma once

#include <cstdint>

namespace axiom {

    class Scene;

    class ISystem {
    public:
        virtual ~ISystem() = default;

        virtual void PreUpdate(Scene& scene, double dt) {}
        virtual void Update(Scene& scene, double dt) {}
        virtual void FixedUpdate(Scene& scene, double dt) {}
        virtual void PostUpdate(Scene& scene, double dt) {}
        virtual void BeginRenderFrame() {}
        virtual void Render(Scene& scene, double alpha) {}
        virtual void OnViewportResize(uint32_t width, uint32_t height) {}
    };

} // namespace axiom

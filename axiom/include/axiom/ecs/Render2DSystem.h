#pragma once

#include "ISystem.h"

namespace axiom {

    class Render2DSystem : public ISystem {
    public:
        void Render(Scene& scene, double alpha) override;
    };

} // namespace axiom

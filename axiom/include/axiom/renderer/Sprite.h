#pragma once
#include <memory>
#include <glm/glm.hpp>
#include "Texture2D.h"

namespace axiom {

    struct Sprite {
        std::shared_ptr<Texture2D> Texture;
        glm::vec2 UVMin{0.0f, 0.0f};
        glm::vec2 UVMax{1.0f, 1.0f};
        glm::vec2 Pivot{0.5f, 0.5f}; // für korrektes Rotieren

        Sprite() = default;
        Sprite(const std::shared_ptr<Texture2D>& texture)
            : Texture(texture) {
        }
        Sprite(const std::shared_ptr<Texture2D>& texture,
               const glm::vec2& uvMin,
               const glm::vec2& uvMax)
            : Texture(texture), UVMin(uvMin), UVMax(uvMax) {
        }
    };

} // namespace axiom

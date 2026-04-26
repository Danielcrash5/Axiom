#pragma once

#include "axiom/core/UUID.h"
#include "axiom/renderer/Sprite.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <string>
#include <utility>

namespace axiom {

    struct IDComponent {
        UUID ID{};

        IDComponent() = default;
        explicit IDComponent(UUID id)
            : ID(id) {
        }
    };

    struct TagComponent {
        std::string Tag = "Entity";

        TagComponent() = default;
        explicit TagComponent(std::string tag)
            : Tag(std::move(tag)) {
        }
    };

    struct TransformComponent {
        glm::vec3 Translation{0.0f, 0.0f, 0.0f};
        glm::vec3 Rotation{0.0f, 0.0f, 0.0f};
        glm::vec3 Scale{1.0f, 1.0f, 1.0f};

        TransformComponent() = default;
        explicit TransformComponent(const glm::vec3& translation)
            : Translation(translation) {
        }

        glm::mat4 GetTransform() const {
            const glm::mat4 rotation =
                glm::rotate(glm::mat4(1.0f), Rotation.x, glm::vec3(1.0f, 0.0f, 0.0f)) *
                glm::rotate(glm::mat4(1.0f), Rotation.y, glm::vec3(0.0f, 1.0f, 0.0f)) *
                glm::rotate(glm::mat4(1.0f), Rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));

            return glm::translate(glm::mat4(1.0f), Translation) *
                   rotation *
                   glm::scale(glm::mat4(1.0f), Scale);
        }
    };

    struct SpriteRendererComponent {
        glm::vec4 Color{1.0f, 1.0f, 1.0f, 1.0f};
        Sprite SpriteData{};
        float TilingFactor = 1.0f;

        SpriteRendererComponent() = default;
        explicit SpriteRendererComponent(const glm::vec4& color)
            : Color(color) {
        }

        SpriteRendererComponent(const Sprite& sprite, const glm::vec4& color = glm::vec4(1.0f), float tilingFactor = 1.0f)
            : Color(color), SpriteData(sprite), TilingFactor(tilingFactor) {
        }

        bool HasTexture() const {
            return static_cast<bool>(SpriteData.Texture);
        }
    };

    struct CircleRendererComponent {
        glm::vec4 Color{1.0f, 1.0f, 1.0f, 1.0f};
        float Thickness = 1.0f;

        CircleRendererComponent() = default;
        CircleRendererComponent(const glm::vec4& color, float thickness = 1.0f)
            : Color(color), Thickness(thickness) {
        }
    };

} // namespace axiom

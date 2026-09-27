#pragma once

#include "axiom/core/TaggedUUID.h"
// #include "axiom/renderer/Sprite.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include <string>
#include <utility>

namespace axiom {

    struct IDComponent {
        EntityID ID{};

        IDComponent() = default;
        explicit IDComponent(UUID id) : ID(id) {}
    };

    struct TagComponent {
        std::string Tag = "Entity";

        TagComponent() = default;
        explicit TagComponent(std::string tag) : Tag(std::move(tag)) {}
    };

    struct TransformComponent {
        glm::vec3 Translation{0.0f, 0.0f, 0.0f};
        // Quaternion statt Euler: keine Gimbal-Lock-Probleme bei Komposition/
        // Interpolation (Physik, Animation, Gizmo). Editierbare Euler-Winkel
        // fuers Inspector-UI sind ein reiner Darstellungs-Cache dort, nicht
        // Teil dieser Komponente - siehe TODO in InspectorPanel.cpp.
        glm::quat Rotation{1.0f, 0.0f, 0.0f, 0.0f}; // (w,x,y,z) Identitaet
        glm::vec3 Scale{1.0f, 1.0f, 1.0f};

        TransformComponent() = default;
        explicit TransformComponent(const glm::vec3 &translation)
            : Translation(translation) {}

        glm::mat4 GetTransform() const {
            return glm::translate(glm::mat4(1.0f), Translation) *
                   glm::mat4_cast(Rotation) *
                   glm::scale(glm::mat4(1.0f), Scale);
        }
    };

    /*struct SpriteRendererComponent {
        glm::vec4 Color{1.0f, 1.0f, 1.0f, 1.0f};
        Sprite SpriteData{};
        float TilingFactor = 1.0f;

        SpriteRendererComponent() = default;
        explicit SpriteRendererComponent(const glm::vec4& color)
            : Color(color) {
        }

        SpriteRendererComponent(const Sprite& sprite, const glm::vec4& color =
    glm::vec4(1.0f), float tilingFactor = 1.0f) : Color(color),
    SpriteData(sprite), TilingFactor(tilingFactor) {
        }

        bool HasTexture() const {
            return static_cast<bool>(SpriteData.Texture);
        }
    };*/

    struct CircleRendererComponent {
        glm::vec4 Color{1.0f, 1.0f, 1.0f, 1.0f};
        float Thickness = 1.0f;

        CircleRendererComponent() = default;
        CircleRendererComponent(const glm::vec4 &color, float thickness = 1.0f)
            : Color(color), Thickness(thickness) {}
    };

    struct CameraComponent {
        bool Primary = false;
        float OrthographicSize = 180.0f;
        float NearClip = -1.0f;
        float FarClip = 1.0f;

        CameraComponent() = default;
        explicit CameraComponent(bool primary) : Primary(primary) {}

        glm::mat4 GetProjection(float aspectRatio) const {
            const float halfHeight = OrthographicSize * 0.5f;
            const float halfWidth = halfHeight * aspectRatio;
            return glm::ortho(-halfWidth, halfWidth, -halfHeight, halfHeight,
                              NearClip, FarClip);
        }
    };

} // namespace axiom

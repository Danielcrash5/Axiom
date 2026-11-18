/**
 * @file axiom/include/axiom/ecs/Components.h
 * @brief Declares the public interface for the components.
 */

#pragma once

#include "axiom/core/TaggedUUID.h"
// #include "axiom/renderer/Sprite.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include <string>
#include <utility>

namespace axiom {

    /** @brief Defines the interface for the d component. */
    struct IDComponent {
        /** @brief Stores the ID. */
        EntityID ID{};

        /**
         * @brief Creates an instance of ID component.
         */
        IDComponent() = default;
        /**
         * @brief Creates an instance of ID component.
         * @param id The ID used by this operation.
         */
        explicit IDComponent(UUID id) : ID(id) {}
    };

    /** @brief Stores the data for the tag component component. */
    struct TagComponent {
        /** @brief Stores the tag. */
        std::string Tag = "Entity";

        /**
         * @brief Creates an instance of tag component.
         */
        TagComponent() = default;
        /**
         * @brief Creates an instance of tag component.
         * @param tag The tag used by this operation.
         */
        explicit TagComponent(std::string tag) : Tag(std::move(tag)) {}
    };

    /** @brief Stores the data for the transform component component. */
    struct TransformComponent {
        /** @brief Stores the translation. */
        glm::vec3 Translation{0.0f, 0.0f, 0.0f};
        // Quaternion statt Euler: keine Gimbal-Lock-Probleme bei Komposition/
        // Interpolation (Physik, Animation, Gizmo). Editierbare Euler-Winkel
        // fuers Inspector-UI sind ein reiner Darstellungs-Cache dort, nicht
        // Teil dieser Komponente - siehe TODO in InspectorPanel.cpp.
        /** @brief Stores the rotation. */
        glm::quat Rotation{1.0f, 0.0f, 0.0f, 0.0f}; // (w,x,y,z) Identitaet
        /** @brief Stores the scale. */
        glm::vec3 Scale{1.0f, 1.0f, 1.0f};

        /**
         * @brief Creates an instance of transform component.
         */
        TransformComponent() = default;
        /**
         * @brief Creates an instance of transform component.
         * @param translation The translation used by this operation.
         */
        explicit TransformComponent(const glm::vec3 &translation)
            : Translation(translation) {}

        /**
         * @brief Returns the transform.
         * @return The result of the operation.
         */
        glm::mat4 getTransform() const {
            return glm::translate(glm::mat4(1.0f), Translation) *
                   glm::mat4_cast(Rotation) *
                   glm::scale(glm::mat4(1.0f), Scale);
        }
    };

    /** @brief Stores the data for the quad renderer component component. */
    struct QuadRendererComponent {
        /** @brief Stores the color. */
        glm::vec4 Color{1.0f};

        /**
         * @brief Creates an instance of quad renderer component.
         */
        QuadRendererComponent() = default;
        /**
         * @brief Creates an instance of quad renderer component.
         * @param color The color used by this operation.
         */
        explicit QuadRendererComponent(const glm::vec4 &color) : Color(color) {}
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

    /** @brief Stores the data for the circle renderer component component. */
    struct CircleRendererComponent {
        /** @brief Stores the color. */
        glm::vec4 Color{1.0f, 1.0f, 1.0f, 1.0f};
        /** @brief Stores the thickness. */
        float Thickness = 1.0f;

        /**
         * @brief Creates an instance of circle renderer component.
         */
        CircleRendererComponent() = default;
        /**
         * @brief Creates an instance of circle renderer component.
         * @param color The color used by this operation.
         * @param thickness The thickness used by this operation.
         */
        CircleRendererComponent(const glm::vec4 &color, float thickness = 1.0f)
            : Color(color), Thickness(thickness) {}
    };

    /** @brief Stores the data for the camera component component. */
    struct CameraComponent {
        /** @brief Stores the primary. */
        bool Primary = false;
        /** @brief Stores the orthographic size. */
        float OrthographicSize = 180.0f;
        /** @brief Stores the near clip. */
        float NearClip = -1.0f;
        /** @brief Stores the far clip. */
        float FarClip = 1.0f;

        /**
         * @brief Creates an instance of camera component.
         */
        CameraComponent() = default;
        /**
         * @brief Creates an instance of camera component.
         * @param primary The primary used by this operation.
         */
        explicit CameraComponent(bool primary) : Primary(primary) {}

        /**
         * @brief Returns the projection.
         * @param aspectRatio The aspect ratio used by this operation.
         * @return The result of the operation.
         */
        glm::mat4 getProjection(float aspectRatio) const {
            const float halfHeight = OrthographicSize * 0.5f;
            const float halfWidth = halfHeight * aspectRatio;
            return glm::ortho(-halfWidth, halfWidth, -halfHeight, halfHeight,
                              NearClip, FarClip);
        }
    };

} // namespace axiom

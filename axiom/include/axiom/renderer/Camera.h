#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace axiom {

    enum class ProjectionType {
        Perspective,
        Orthographic,
        Custom
    };

    class Camera {
    public:
        Camera();
        ~Camera() = default;

        // ---------------- Projection Setup ----------------
        void setPerspective(float fovDeg, float aspect, float nearPlane, float farPlane);
        void setOrthographic(float left, float right, float bottom, float top, float nearPlane, float farPlane);
        void setCustomProjection(const glm::mat4& proj);

        // ---------------- Transform ----------------
        void setPosition(const glm::vec3& pos);
        void setRotation(const glm::quat& rot);
        void lookAt(const glm::vec3& target, const glm::vec3& up = glm::vec3(0, 1, 0));

        // ---------------- Getters ----------------
        const glm::mat4& getViewMatrix() const;
        const glm::mat4& getProjectionMatrix() const;
        const glm::mat4& getViewProjectionMatrix() const;

        ProjectionType getProjectionType() const {
            return m_ProjectionType;
        }

    private:
        void recalcViewMatrix();
        void recalcViewProjectionMatrix();

    private:
        glm::vec3 m_Position = { 0.0f, 0.0f, 0.0f };
        glm::quat m_Rotation = { 1.0f, 0.0f, 0.0f, 0.0f };

        glm::mat4 m_ViewMatrix = glm::mat4(1.0f);
        glm::mat4 m_ProjectionMatrix = glm::mat4(1.0f);
        glm::mat4 m_ViewProjectionMatrix = glm::mat4(1.0f);

        ProjectionType m_ProjectionType = ProjectionType::Perspective;
    };
}
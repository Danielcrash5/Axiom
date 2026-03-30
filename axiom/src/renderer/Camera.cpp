#include "axiom/renderer/Camera.h"

namespace axiom {

    // ---------------- Constructor ----------------
    Camera::Camera() {
        recalcViewMatrix();
        recalcViewProjectionMatrix();
    }

    // ---------------- Projection Setup ----------------
    void Camera::setPerspective(float fovDeg, float aspect, float nearPlane, float farPlane) {
        m_ProjectionMatrix = glm::perspective(glm::radians(fovDeg), aspect, nearPlane, farPlane);
        m_ProjectionType = ProjectionType::Perspective;
        recalcViewProjectionMatrix();
    }

    void Camera::setOrthographic(float left, float right, float bottom, float top, float nearPlane, float farPlane) {
        m_ProjectionMatrix = glm::ortho(left, right, bottom, top, nearPlane, farPlane);
        m_ProjectionType = ProjectionType::Orthographic;
        recalcViewProjectionMatrix();
    }

    void Camera::setCustomProjection(const glm::mat4& proj) {
        m_ProjectionMatrix = proj;
        m_ProjectionType = ProjectionType::Custom;
        recalcViewProjectionMatrix();
    }

    // ---------------- Transform ----------------
    void Camera::setPosition(const glm::vec3& pos) {
        m_Position = pos;
        recalcViewMatrix();
    }

    void Camera::setRotation(const glm::quat& rot) {
        m_Rotation = rot;
        recalcViewMatrix();
    }

    void Camera::lookAt(const glm::vec3& target, const glm::vec3& up) {
        m_ViewMatrix = glm::lookAt(m_Position, target, up);
        recalcViewProjectionMatrix();
    }

    // ---------------- Internal ----------------
    void Camera::recalcViewMatrix() {
        glm::mat4 rot = glm::mat4_cast(m_Rotation);
        glm::mat4 trans = glm::translate(glm::mat4(1.0f), -m_Position);

        m_ViewMatrix = rot * trans;
        recalcViewProjectionMatrix();
    }

    void Camera::recalcViewProjectionMatrix() {
        m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
    }

    // ---------------- Getters ----------------
    const glm::mat4& Camera::getViewMatrix() const {
        return m_ViewMatrix;
    }
    const glm::mat4& Camera::getProjectionMatrix() const {
        return m_ProjectionMatrix;
    }
    const glm::mat4& Camera::getViewProjectionMatrix() const {
        return m_ViewProjectionMatrix;
    }

}
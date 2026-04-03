#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace axiom {
    class Camera {
    public:
        void SetOrthographic(float left, float right, float bottom, float top, float near, float far) {
            m_Projection = glm::ortho(left, right, bottom, top, near, far);
        }

        void SetPerspective(float fov, float aspect, float near, float far) {
            m_Projection = glm::perspective(glm::radians(fov), aspect, near, far);
        }

        void SetPosition(const glm::vec3& position) {
            m_Position = position; RecalculateView();
        }
        void SetRotation(const glm::vec3& rotation) {
            m_Rotation = rotation; RecalculateView();
        }

        const glm::mat4& GetProjection() const {
            return m_Projection;
        }
        const glm::mat4& GetView() const {
            return m_View;
        }
        glm::mat4 GetViewProjection() const {
            return m_Projection * m_View;
        }

    private:
        void RecalculateView() {
            glm::mat4 rot = glm::rotate(glm::mat4(1.0f), m_Rotation.x, glm::vec3(1, 0, 0));
            rot = glm::rotate(rot, m_Rotation.y, glm::vec3(0, 1, 0));
            rot = glm::rotate(rot, m_Rotation.z, glm::vec3(0, 0, 1));
            glm::mat4 transform =
                glm::translate(glm::mat4(1.0f), m_Position) * rot;
            m_View = glm::inverse(transform);
        }

    private:
        glm::mat4 m_Projection { 1.0f };
        glm::mat4 m_View { 1.0f };
        glm::vec3 m_Position { 0.0f };
        glm::vec3 m_Rotation { 0.0f };
    };

}
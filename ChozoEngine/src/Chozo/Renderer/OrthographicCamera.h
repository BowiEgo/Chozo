#pragma once

#include <glm/glm.hpp>

namespace Chozo {

    class OrthographicCamera
    {
    public:
        OrthographicCamera(const float width, const float height);

        const glm::vec3& GetPosition() const { return m_Position; }
        void Zoom(const float zoomLevel);
        void SetPosition(const glm::vec3& position) { m_Position = position; RecalculateViewMatrix(); }

        float GetRotation() const { return m_Rotation; }
        void SetRotation(const float rotation) { m_Rotation = rotation; RecalculateViewMatrix(); }
        void Resize();
        void Resize(const float width, const float height);

        const glm::mat4& GetProjectionMatrix() const { return m_ProjectionMatrix; }
        const glm::mat4& GetViewMatrix() const { return m_ViewMatrix; }
        const glm::mat4& GetViewProjectionMatrix() const { return m_ViewProjectionMatrix; }
    private:
        void RecalculateViewMatrix();
    private:
        glm::mat4 m_ProjectionMatrix;
        glm::mat4 m_ViewMatrix;
        glm::mat4 m_ViewProjectionMatrix;

        float m_AspectRatio;
        float m_ZoomLevel = 1.0f;
        glm::vec3 m_Position = { 0.0f, 0.0f, 0.0f };
        float m_Rotation = 0.0f;
    };
}
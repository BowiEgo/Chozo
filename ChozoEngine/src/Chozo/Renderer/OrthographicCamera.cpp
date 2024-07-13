#include "OrthographicCamera.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Chozo {

    OrthographicCamera::OrthographicCamera(const float width, const float height)
        : m_Width(width), m_Height(height), m_ViewMatrix(1.0f)
    {
        Resize(m_Width, m_Height);
    }

    void OrthographicCamera::Zoom(const float zoomLevel)
    {
         m_ZoomLevel = zoomLevel;
         Resize(m_Width, m_Height);
    }

    void OrthographicCamera::Resize(const float width, const float height)
    {
        m_Width = width;
        m_Height = height;
        m_ProjectionMatrix = glm::ortho(-0.5f * width * m_ZoomLevel * 0.01f, 0.5f * width * m_ZoomLevel * 0.01f, -0.5f * height * m_ZoomLevel * 0.01f, 0.5f * height * m_ZoomLevel * 0.01f, -1.0f, 1.0f);
        m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
    }

    void OrthographicCamera::RecalculateViewMatrix()
    {
        glm::mat4 transform = glm::translate(glm::mat4(1.0f), m_Position)
            * glm::rotate(glm::mat4(1.0f), glm::radians(m_Rotation), glm::vec3(0, 0, 1));

        m_ViewMatrix = glm::inverse(transform);
        m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
    }
}
#include "OrthographicCamera.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Chozo {

    OrthographicCamera::OrthographicCamera(const float width, const float height)
        : m_ViewMatrix(1.0f)
    {
        Resize(width, height);
    }

    void OrthographicCamera::Zoom(const float zoomLevel)
    {
         m_ZoomLevel = zoomLevel;
         Resize();
    }

    void OrthographicCamera::Resize()
    {
        m_ProjectionMatrix = glm::ortho(-m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel, -1.0f, 1.0f);
        m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
    }

    void OrthographicCamera::Resize(const float width, const float height)
    {
        m_AspectRatio = width / height;
        Resize();
    }

    void OrthographicCamera::RecalculateViewMatrix()
    {
        glm::mat4 transform = glm::translate(glm::mat4(1.0f), m_Position)
            * glm::rotate(glm::mat4(1.0f), glm::radians(m_Rotation), glm::vec3(0, 0, 1));

        m_ViewMatrix = glm::inverse(transform);
        m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
    }
}
#include "SceneCamera.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Chozo
{
    SceneCamera::SceneCamera()
    {
        RecaculateProjection();
    }

    void SceneCamera::SetOrthographic(float size, float nearClip, float farClip)
    {
        m_OrthograhicSize = size;
        m_OrthograhicNear = nearClip;
        m_OrthograhicFar = farClip;

        RecaculateProjection();
    }

    void SceneCamera::SetViewportSize(uint32_t width, uint32_t height)
    {
        m_AspectRatio = (float)width / (float)height;

        RecaculateProjection();
    }

    void SceneCamera::RecaculateProjection()
    {
        float orthoLeft = -m_OrthograhicSize * m_AspectRatio * 0.5f;
        float orthoRight = m_OrthograhicSize * m_AspectRatio * 0.5f;
        float orthoBottom = -m_OrthograhicSize * 0.5f;
        float orthoTop = m_OrthograhicSize * 0.5f;

        m_ProjectionMatrix = glm::ortho(orthoLeft, orthoRight, orthoBottom, orthoTop,
            m_OrthograhicNear, m_OrthograhicFar);
    }
}

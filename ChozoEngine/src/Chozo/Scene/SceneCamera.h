#pragma once

#include "Chozo/Renderer/Camera.h"

namespace Chozo
{
    
    class SceneCamera : public Camera
    {
    public:
        SceneCamera();
        virtual ~SceneCamera() = default;

        void SetOrthographic(float size, float nearClip, float farClip);
        void SetViewportSize(uint32_t width, uint32_t height);

        inline void SetOrthographicSize(float size) { m_OrthograhicSize = size; RecaculateProjection();};
        inline float GetOrthographicSize() const { return m_OrthograhicSize; };
    private:
        void RecaculateProjection();
    private:
        float m_AspectRatio = 0.0f;
        float m_OrthograhicSize = 10.0f;
        float m_OrthograhicNear = -1.0f, m_OrthograhicFar = 1.0f;
        uint32_t m_ViewportWidth = 0.0f, m_ViewportHeight = 0.0f;
    };
}

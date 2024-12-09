#pragma once

#include "Chozo/Renderer/Camera.h"

namespace Chozo
{
    
    class SceneCamera : public Camera
    {
    public:
        enum class ProjectionType { Perspective = 0, Orthographic = 1 };
    public:
        SceneCamera();
        virtual ~SceneCamera() = default;

        virtual void Reset() override;

        inline void SetProjectionType(ProjectionType type) { m_ProjectionType = type; RecaculateProjection(); }
        inline ProjectionType GetProjectionType() const { return m_ProjectionType; }
        void SetViewportSize(float width, float height);

        void SetPerspective(float verticalFOV, float nearClip, float farClip);
        inline void SetPerspectiveVerticalFOV(float fov) { m_PerspectiveFOV = fov; RecaculateProjection(); };
        inline float GetPerspectiveVerticalFOV() const { return m_PerspectiveFOV; }
        inline void SetPerspectiveNearClip(float nearClip) { m_PerspectiveNear = nearClip; RecaculateProjection(); }
        inline float GetPerspectiveNearClip() const { return m_PerspectiveNear; }
        inline void SetPerspectiveFarClip(float farClip) { m_PerspectiveFar = farClip; RecaculateProjection(); }
        inline float GetPerspectiveFarClip() const { return m_PerspectiveFar; }

        void SetOrthographic(float size, float nearClip, float farClip);
        inline void SetOrthographicSize(float size) { m_OrthograhicSize = size; RecaculateProjection(); };
        inline float GetOrthographicSize() const { return m_OrthograhicSize; }
        inline void SetOrthographicNearClip(float nearClip) { m_OrthograhicNear = nearClip; RecaculateProjection(); }
        inline float GetOrthographicNearClip() const { return m_OrthograhicNear; }
        inline void SetOrthographicFarClip(float farClip) { m_OrthograhicFar = farClip; RecaculateProjection(); }
        inline float GetOrthographicFarClip() const { return m_OrthograhicFar; }
    private:
        void RecaculateProjection();
    private:
        ProjectionType m_ProjectionType = ProjectionType::Perspective;

        float m_PerspectiveFOV = glm::radians(45.0f);
        float m_PerspectiveNear = 0.01f, m_PerspectiveFar = 1000.0f;

        float m_OrthograhicSize = 10.0f;
        float m_OrthograhicNear = -1.0f, m_OrthograhicFar = 1.0f;

        float m_AspectRatio = 0.0f;
    };
}

#pragma once

#include "CoreMinimal.h"
#include "Matrix4.h"
#include "Quaternion.h"
#include "Ref.h"
#include "Vector3.h"

#include "RenderCoreExport.h"

enum class ECameraProjectionMode { Perspective, Orthographic };

class RENDER_CORE_API CCamera : public FRefCounted {
    friend class CEditorCamera;

public:
    CCamera();
    CCamera(float fov, float aspectRatio, float nearClip, float farClip);
    ~CCamera();

    void SetPerspective(float fovDegrees, float aspectRatio, float nearClip, float farClip);
    void SetOrthographic(float nearClip, float farClip);

    void Reset();
    void SetDistance(float distance) { m_Distance = distance; }
    void SetViewportSize(float width, float height);
    void SetPosition(const FVector3& position);
    void SetRotation(const FVector3& rotation);
    void LookAt(const FVector3& target, const FVector3& up = FVector3(0, 1, 0));

    float GetFOV() const { return m_FOV; }
    float GetAspectRatio() const { return m_AspectRatio; }
    float GetNearClip() const { return m_NearClip; }
    float GetFarClip() const { return m_FarClip; }
    float GetDistance() const { return m_Distance; };
    FMatrix4 GetViewMatrix() const;
    FMatrix4 GetProjectionMatrix() const;
    FMatrix4 GetViewProjectionMatrix() const { return GetViewMatrix() * GetProjectionMatrix(); }

    const FVector3& GetPosition() const { return m_Position; }
    const FVector3& GetRotation() const { return m_Rotation; }
    FQuaternion GetOrientation() const { return FQuaternion::FromEuler(m_Rotation); }
    FVector3 GetUpDirection() const { return GetOrientation() * FVector3::Up; }
    FVector3 GetRightDirection() const { return GetOrientation() * FVector3::Right; }
    FVector3 GetForwardDirection() const { return GetOrientation() * FVector3::Forward; }

    void Resize();
    void Resize(uint32_t width, uint32_t height);
    void Zoom(const float zoomLevel) {
        m_ZoomLevel = zoomLevel;
        Resize();
    }

private:
    void RecalculatePosition();
    void RecalculateViewMatrix();
    void RecalculateProjectionMatrix();

protected:
    ECameraProjectionMode m_ProjectionMode = ECameraProjectionMode::Perspective;
    float m_ViewportWidth = 1280, m_ViewportHeight = 720;
    float m_ZoomLevel = 1.0f;
    float m_FOV = 45.0f, m_AspectRatio = 1.778f, m_NearClip = 0.1f, m_FarClip = 1000.0f;

    FVector3 m_Position = { 0.0f, 0.0f, 0.0f };
    FVector3 m_Rotation = { 0.0f, 0.0f, 0.0f };
    FVector3 m_FocalPoint = { 0.0f, 0.0f, 0.0f };

    float m_Distance = 5.0f;

    FMatrix4 m_ViewMatrix;
    FMatrix4 m_ProjectionMatrix;
    bool m_ViewDirty = true, m_ProjDirty = true;
};
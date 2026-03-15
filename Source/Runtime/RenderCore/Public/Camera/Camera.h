#pragma once

#include "CoreMinimal.h"
#include "Matrix4.h"
#include "RenderCoreExport.h"
#include "Vector3.h"

enum class ECameraProjectionMode { Perspective, Orthographic };

class RENDER_CORE_API CCamera {
public:
    CCamera();
    ~CCamera() = default;

    void SetPerspective(float fovDegrees, float aspectRatio, float nearPlane, float farPlane);
    void SetOrthographic(float left, float right, float bottom, float top, float nearPlane,
                         float farPlane);

    void SetPosition(const FVector3& position);
    void SetRotation(const FVector3& rotation);
    void LookAt(const FVector3& target, const FVector3& up = FVector3(0, 1, 0));

    FMatrix4 GetViewMatrix() const;
    FMatrix4 GetProjectionMatrix() const;
    FMatrix4 GetViewProjectionMatrix() const { return GetViewMatrix() * GetProjectionMatrix(); }

    void OnResize(uint32_t width, uint32_t height);

    void Update(float deltaTime);

private:
    void RecalculateMatrices();

private:
    ECameraProjectionMode m_ProjectionMode = ECameraProjectionMode::Perspective;
    float m_FOV = 60.0f;
    float m_AspectRatio = 16.0f / 9.0f;
    float m_NearPlane = 0.1f;
    float m_FarPlane = 1000.0f;

    float m_Left = -10.0f;
    float m_Right = 10.0f;
    float m_Bottom = -10.0f;
    float m_Top = 10.0f;

    FVector3 m_Position = FVector3(0, 0, 5);
    FVector3 m_Rotation = FVector3(0, 0, 0); // pitch, yaw, roll

    FMatrix4 m_ViewMatrix;
    FMatrix4 m_ProjectionMatrix;
    bool m_bDirty = true;
};
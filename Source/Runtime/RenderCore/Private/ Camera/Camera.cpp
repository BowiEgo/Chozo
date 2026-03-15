#include "Camera.h"

CCamera::CCamera() { RecalculateMatrices(); }

void CCamera::SetPerspective(float fovDegrees, float aspectRatio, float nearPlane, float farPlane) {
    m_ProjectionMode = ECameraProjectionMode::Perspective;
    m_FOV = fovDegrees;
    m_AspectRatio = aspectRatio;
    m_NearPlane = nearPlane;
    m_FarPlane = farPlane;
    m_bDirty = true;
}

void CCamera::SetOrthographic(float left, float right, float bottom, float top, float nearPlane,
                              float farPlane) {
    m_ProjectionMode = ECameraProjectionMode::Orthographic;
    m_Left = left;
    m_Right = right;
    m_Bottom = bottom;
    m_Top = top;
    m_NearPlane = nearPlane;
    m_FarPlane = farPlane;
    m_bDirty = true;
}

void CCamera::SetPosition(const FVector3& position) {
    m_Position = position;
    m_bDirty = true;
}

void CCamera::SetRotation(const FVector3& rotation) {
    m_Rotation = rotation;
    m_bDirty = true;
}

void CCamera::LookAt(const FVector3& target, const FVector3& up) {
    // Implement LookAt logic
    FVector3 direction = (target - m_Position).Normalized();
    // Simplified here, actual implementation needs to calculate Euler angles or directly construct
    // view matrix For simple projects, Euler angles control can be used initially
    m_bDirty = true;
}

FMatrix4 CCamera::GetViewMatrix() const {
    if (m_bDirty) {
        const_cast<CCamera*>(this)->RecalculateMatrices();
    }
    return m_ViewMatrix;
}

FMatrix4 CCamera::GetProjectionMatrix() const {
    if (m_bDirty) {
        const_cast<CCamera*>(this)->RecalculateMatrices();
    }
    return m_ProjectionMatrix;
}

void CCamera::RecalculateMatrices() {
    // View matrix: constructed from position and rotation
    // Using simplified LookAt, assuming camera looks at -Z direction
    FVector3 target = m_Position + FVector3(0, 0, -1);
    FVector3 up = FVector3(0, 1, 0);

    // Manually construct LookAt matrix
    FVector3 forward = (target - m_Position).Normalized();
    FVector3 right = up.Cross(forward).Normalized();
    FVector3 newUp = forward.Cross(right);

    m_ViewMatrix = FMatrix4(right.x, newUp.x, forward.x, 0, right.y, newUp.y, forward.y, 0, right.z,
                            newUp.z, forward.z, 0, -right.Dot(m_Position), -newUp.Dot(m_Position),
                            -forward.Dot(m_Position), 1);

    // Projection matrix
    if (m_ProjectionMode == ECameraProjectionMode::Perspective) {
        float tanHalfFOV = tanf(m_FOV * 0.5f * M_PI / 180.0f);
        m_ProjectionMatrix = m_ProjectionMatrix =
            FMatrix4(1.0f / (m_AspectRatio * tanHalfFOV), 0, 0, 0, 0, -1.0f / tanHalfFOV, 0, 0, 0,
                     0, m_FarPlane / (m_FarPlane - m_NearPlane), 1, 0, 0,
                     -(m_NearPlane * m_FarPlane) / (m_FarPlane - m_NearPlane), 0);
    } else {
        // Orthographic projection
        m_ProjectionMatrix = FMatrix4(
            2.0f / (m_Right - m_Left), 0, 0, 0, 0, -2.0f / (m_Top - m_Bottom), 0, 0, 0, 0,
            1.0f / (m_FarPlane - m_NearPlane), 0, -(m_Right + m_Left) / (m_Right - m_Left),
            -(m_Top + m_Bottom) / (m_Top - m_Bottom), -m_NearPlane / (m_FarPlane - m_NearPlane), 1);
    }

    m_bDirty = false;
}

void CCamera::OnResize(uint32_t width, uint32_t height) {
    m_AspectRatio = (float)width / (float)height;
    m_bDirty = true;
}

void CCamera::Update(float deltaTime) {
    // Camera control (WASD movement, mouse rotation, etc.) can be handled here
    // Currently left empty
}
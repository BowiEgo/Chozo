#include "Camera.h"

#include "CameraUniformManager.h"

CCamera::CCamera() {
    RecalculateViewMatrix();
    RecalculateProjectionMatrix();

    CCameraUniformManager::Get().RegisterCamera(this);
}

CCamera::CCamera(float fov, float aspectRatio, float nearClip, float farClip)
    : m_FOV(fov), m_AspectRatio(aspectRatio), m_NearClip(nearClip), m_FarClip(farClip) {
    RecalculateViewMatrix();
    RecalculateProjectionMatrix();

    CCameraUniformManager::Get().RegisterCamera(this);
}

CCamera::~CCamera() { CCameraUniformManager::Get().UnregisterCamera(this); }

void CCamera::SetPerspective(float fov, float aspectRatio, float nearClip, float farClip) {
    m_ProjectionMode = ECameraProjectionMode::Perspective;
    m_FOV            = fov;
    m_AspectRatio    = aspectRatio;
    m_NearClip       = nearClip;
    m_FarClip        = farClip;
    m_ProjDirty      = true;
}

void CCamera::SetOrthographic(float nearClip, float farClip) {
    m_ProjectionMode = ECameraProjectionMode::Orthographic;
    m_NearClip       = nearClip;
    m_FarClip        = farClip;
    m_ProjDirty      = true;
}

void CCamera::Reset() {
    m_Distance  = 5.0f;
    m_Rotation  = FVector3(0.0f, 0.0f, 0.0f);
    m_ViewDirty = true;
}

void CCamera::SetViewportSize(float width, float height) {
    if (width == 0 || height == 0) return;
    if (m_ViewportWidth == width && m_ViewportHeight == height) return;

    m_ViewportWidth  = width;
    m_ViewportHeight = height;

    m_AspectRatio = m_ViewportWidth / m_ViewportHeight;
    m_ProjDirty   = true;
}

void CCamera::SetPosition(const FVector3& position) {
    m_Position  = position;
    m_ViewDirty = true;
}

void CCamera::SetRotation(const FVector3& rotation) {
    m_Rotation  = rotation;
    m_ViewDirty = true;
}

void CCamera::LookAt(const FVector3& target, const FVector3& up) {
    m_ViewMatrix = FMatrix4::LookAt(m_Position, target, up);
    m_ViewDirty  = false;
}

FMatrix4 CCamera::GetViewMatrix() const {
    if (m_ViewDirty) {
        const_cast<CCamera*>(this)->RecalculateViewMatrix();
    }
    return m_ViewMatrix;
}

FMatrix4 CCamera::GetProjectionMatrix() const {
    if (m_ProjDirty) {
        const_cast<CCamera*>(this)->RecalculateProjectionMatrix();
    }
    return m_ProjectionMatrix;
}

void CCamera::Resize() { Resize(m_ViewportWidth, m_ViewportHeight); }

void CCamera::Resize(uint32_t width, uint32_t height) {
    if (m_ProjectionMode == ECameraProjectionMode::Perspective) {
        m_ViewportWidth  = (float)width;
        m_ViewportHeight = (float)height;
        m_AspectRatio    = m_ViewportWidth / m_ViewportHeight;
    }

    m_ProjDirty = true;
}

void CCamera::RecalculatePosition() {
    m_Position = m_FocalPoint - GetForwardDirection() * m_Distance;
}

void CCamera::RecalculateViewMatrix() {
    RecalculatePosition();

    m_ViewMatrix = FMatrix4::LookAt(m_Position, m_FocalPoint, GetUpDirection());
    m_ViewDirty  = false;
}

void CCamera::RecalculateProjectionMatrix() {
    if (m_ProjectionMode == ECameraProjectionMode::Perspective) {
        m_ProjectionMatrix = FMatrix4::Perspective(m_FOV, m_AspectRatio, m_NearClip, m_FarClip);
    } else {
        float left   = -m_AspectRatio * m_ZoomLevel;
        float right  = m_AspectRatio * m_ZoomLevel;
        float bottom = -m_ZoomLevel;
        float top    = m_ZoomLevel;

        m_ProjectionMatrix =
            FMatrix4::Orthographic(left, right, bottom, top, m_NearClip, m_FarClip);
    }

    m_ProjDirty = false;
}

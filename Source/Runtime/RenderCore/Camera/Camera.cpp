#include <Runtime/RenderCore/Camera/Camera.hpp>
#include <Runtime/RenderCore/Camera/CameraManager.hpp>

namespace CZ {

template <> void Handle<CameraObj>::Destroy() {
    if (m_Obj) {
        CameraManager::Get().UnregisterCamera(m_Obj);

        Delete(m_Obj);
        m_Obj = nullptr;
    }
}

CameraObj::CameraObj() {
    RecalculateViewMatrix();
    RecalculateProjectionMatrix();

    CameraManager::Get().RegisterCamera(this);
}

CameraObj::CameraObj(float fov, float aspectRatio, float nearClip, float farClip)
    : m_FOV(fov), m_AspectRatio(aspectRatio), m_NearClip(nearClip), m_FarClip(farClip) {
    RecalculateViewMatrix();
    RecalculateProjectionMatrix();

    CameraManager::Get().RegisterCamera(this);
}

CameraObj::~CameraObj() {}

void CameraObj::SetPerspective(float fov, float aspectRatio, float nearClip, float farClip) {
    m_ProjectionMode = CameraProjectionMode::Perspective;
    m_FOV            = fov;
    m_AspectRatio    = aspectRatio;
    m_NearClip       = nearClip;
    m_FarClip        = farClip;
    m_ProjDirty      = true;
}

void CameraObj::SetOrthographic(float nearClip, float farClip) {
    m_ProjectionMode = CameraProjectionMode::Orthographic;
    m_NearClip       = nearClip;
    m_FarClip        = farClip;
    m_ProjDirty      = true;
}

void CameraObj::Reset() {
    m_Distance  = 5.0f;
    m_Rotation  = Vector3(0.0f, 0.0f, 0.0f);
    m_ViewDirty = true;
}

void CameraObj::SetViewportSize(float width, float height) {
    if (width == 0 || height == 0) return;
    if (m_ViewportWidth == width && m_ViewportHeight == height) return;

    m_ViewportWidth  = width;
    m_ViewportHeight = height;

    m_AspectRatio = m_ViewportWidth / m_ViewportHeight;
    m_ProjDirty   = true;
}

void CameraObj::SetPosition(const Vector3& position) {
    m_Position  = position;
    m_ViewDirty = true;
}

void CameraObj::SetRotation(const Vector3& rotation) {
    m_Rotation  = rotation;
    m_ViewDirty = true;
}

void CameraObj::MoveRotation(const Vector2& delta) {
    m_Rotation.x += delta.x;
    m_Rotation.y += delta.y;
    m_Rotation.x = std::clamp(m_Rotation.x, -89.0f, 89.0f);
    m_ViewDirty  = true;
}

void CameraObj::SetFocalPoint(const Vector3& focalPoint) {
    m_FocalPoint = focalPoint;
    m_ViewDirty  = true;
}

void CameraObj::MoveFocalPoint(const Vector3& translation) {
    m_FocalPoint += translation;
    m_ViewDirty = true;
}

void CameraObj::LookAt(const Vector3& target, const Vector3& up) {
    m_ViewMatrix = Matrix4::LookAt(m_Position, target, up);
    m_ViewDirty  = false;
}

Matrix4 CameraObj::GetViewMatrix() {
    if (m_ViewDirty) {
        RecalculateViewMatrix();
    }
    return m_ViewMatrix;
}

Matrix4 CameraObj::GetProjectionMatrix() {
    if (m_ProjDirty) {
        RecalculateProjectionMatrix();
    }
    return m_ProjectionMatrix;
}

void CameraObj::Resize() { Resize(m_ViewportWidth, m_ViewportHeight); }

void CameraObj::Resize(uint32_t width, uint32_t height) {
    if (m_ProjectionMode == CameraProjectionMode::Perspective) {
        m_ViewportWidth  = (float)width;
        m_ViewportHeight = (float)height;
        m_AspectRatio    = m_ViewportWidth / m_ViewportHeight;
    }

    m_ProjDirty = true;
}

void CameraObj::RecalculatePosition() {
    m_Position = m_FocalPoint + GetForwardDirection() * m_Distance;
}

void CameraObj::RecalculateViewMatrix() {
    RecalculatePosition();
    Vector3 target = m_Position + GetForwardDirection();
    Vector3 up     = GetUpDirection();

    m_ViewMatrix = Matrix4::LookAt(m_Position, target, up);

    m_ViewDirty = false;
}

void CameraObj::RecalculateProjectionMatrix() {
    if (m_ProjectionMode == CameraProjectionMode::Perspective) {
        m_ProjectionMatrix = Matrix4::Perspective(m_FOV, m_AspectRatio, m_NearClip, m_FarClip);
    } else {
        float left   = -m_AspectRatio * m_ZoomLevel;
        float right  = m_AspectRatio * m_ZoomLevel;
        float bottom = -m_ZoomLevel;
        float top    = m_ZoomLevel;

        m_ProjectionMatrix = Matrix4::Orthographic(left, right, bottom, top, m_NearClip, m_FarClip);
    }

    m_ProjDirty = false;
}

} // namespace CZ
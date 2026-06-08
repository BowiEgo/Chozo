#pragma once

#include <Core/Header/Handle.hpp>
#include <Core/Math/Matrix4.hpp>
#include <Core/Math/Quaternion.hpp>
#include <Core/Math/Vector2.hpp>
#include <Core/Math/Vector3.hpp>

namespace CZ {

enum class CameraProjectionMode { Perspective, Orthographic };

class CameraObj {
public:
    CameraObj();
    CameraObj(float fov, float aspectRatio, float nearClip, float farClip);
    ~CameraObj();

    void SetPerspective(float fovDegrees, float aspectRatio, float nearClip, float farClip);
    void SetOrthographic(float nearClip, float farClip);

    void Reset();
    void SetDistance(float distance) { m_Distance = distance; }
    void MoveDistance(float delta) { m_Distance += delta; }
    void SetViewportSize(float width, float height);
    void SetPosition(const Vector3& position);
    void SetRotation(const Vector3& rotation);
    void MoveRotation(const Vector2& delta);
    void SetFocalPoint(const Vector3& focalPoint);
    void MoveFocalPoint(const Vector3& translation);
    void LookAt(const Vector3& target, const Vector3& up = Vector3(0, 1, 0));

    float GetFOV() const { return m_FOV; }
    float GetAspectRatio() const { return m_AspectRatio; }
    float GetNearClip() const { return m_NearClip; }
    float GetFarClip() const { return m_FarClip; }
    float GetDistance() const { return m_Distance; };
    float GetViewportWidth() const { return m_ViewportWidth; }
    float GetViewportHeight() const { return m_ViewportHeight; }
    Matrix4 GetViewMatrix();
    Matrix4 GetProjectionMatrix();
    Matrix4 GetViewProjectionMatrix() { return GetViewMatrix() * GetProjectionMatrix(); }

    const Vector3& GetPosition() const { return m_Position; }
    const Vector3& GetRotation() const { return m_Rotation; }
    Quaternion GetOrientation() const { return Quaternion::FromEuler(m_Rotation); }
    Vector3 GetUpDirection() const { return GetOrientation() * Vector3::Up; }
    Vector3 GetRightDirection() const { return GetOrientation() * Vector3::Right; }
    Vector3 GetForwardDirection() const { return GetOrientation() * Vector3::Forward; }

    void Resize();
    void Resize(uint32_t width, uint32_t height);
    void Zoom(const float zoomLevel) {
        m_ZoomLevel = zoomLevel;
        Resize();
    }

    void MarkViewDirty() { m_ViewDirty = true; }
    void MarkProjectionDirty() { m_ProjDirty = true; }

private:
    void RecalculatePosition();
    void RecalculateViewMatrix();
    void RecalculateProjectionMatrix();

protected:
    CameraProjectionMode m_ProjectionMode = CameraProjectionMode::Perspective;
    float m_ViewportWidth = 1280, m_ViewportHeight = 720;
    float m_ZoomLevel = 1.0f;
    float m_FOV = 45.0f, m_AspectRatio = 1.778f, m_NearClip = 0.1f, m_FarClip = 1000.0f;

    Vector3 m_Position   = { 0.0f, 0.0f, 0.0f };
    Vector3 m_Rotation   = { 0.0f, 0.0f, 0.0f };
    Vector3 m_FocalPoint = { 0.0f, 0.0f, 0.0f };

    float m_Distance = 5.0f;

    Matrix4 m_ViewMatrix;
    Matrix4 m_ProjectionMatrix;
    bool m_ViewDirty = true, m_ProjDirty = true;
};

class Camera : public Handle<class CameraObj> {
public:
    using Handle<class CameraObj>::Handle;

    CameraObj* Raw() { return InternalHandleReader::Unwrap(*this); }
};

} // namespace CZ
#pragma once

#include <Core/Event/MouseEvent.hpp>
#include <Core/Math/Vector2.hpp>
#include <Runtime/RenderCore/Camera/SceneCamera.hpp>

using namespace CZ;

DECLARE_LOG_CATEGORY_EXTERN(LogEditorCamera, Info);

class EditorCamera {
public:
    EditorCamera() = default;
    EditorCamera(float fov, float aspectRatio, float nearClip, float farClip);

    void OnUpdate(float deltaTime, bool bUpdateInput);
    void OnEvent(Event& e);
    void SetActiveCamera(SceneCamera camera) { m_ActiveCamera = camera; }
    void CopyTo(SceneCamera target) const {
        if (m_ActiveCamera && m_ActiveCamera != target) target.SyncFrom(m_ActiveCamera);
    }

    SceneCamera GetActiveCamera() { return m_ActiveCamera; }

private:
    bool OnMouseScroll(MouseScrolledEvent& e);

    void MousePan(const Vector2& delta);
    void MouseRotate(const Vector2& delta);
    void MouseZoom(float delta);

    std::pair<float, float> PanSpeed() const;
    float RotationSpeed() const;
    float ZoomSpeed() const;

private:
    SceneCamera m_ActiveCamera;
    Vector2 m_InitialMousePosition;
};
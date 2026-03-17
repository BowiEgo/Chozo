#pragma once

#include "Event.h"
#include "MouseEvent.h"
#include "Quaternion.h"
#include "Ref.h"
#include "SceneCamera.h"
#include "Vector2.h"

#include "RenderCoreExport.h"

DECLARE_LOG_CATEGORY_EXTERN(LogEditorCamera, Info);

class RENDER_CORE_API CEditorCamera {
public:
    CEditorCamera() = default;
    CEditorCamera(float fov, float aspectRatio, float nearClip, float farClip);

    void OnUpdate(float deltaTime);
    void OnEvent(IEvent& e);
    void SetActiveCamera(TRef<CSceneCamera> camera) { m_ActiveCamera = camera; }
    void CopyTo(TRef<CSceneCamera> target) const {
        if (m_ActiveCamera && m_ActiveCamera != target) target->SyncFrom(m_ActiveCamera);
    }

    TRef<CSceneCamera> GetActiveCamera() { return m_ActiveCamera; }

private:
    bool OnMouseScroll(FMouseScrolledEvent& e);

    void MousePan(const FVector2& delta);
    void MouseRotate(const FVector2& delta);
    void MouseZoom(float delta);

    std::pair<float, float> PanSpeed() const;
    float RotationSpeed() const;
    float ZoomSpeed() const;

private:
    TRef<CSceneCamera> m_ActiveCamera;
    FVector2 m_InitialMousePosition;
};
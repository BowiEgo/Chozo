#include "EditorCamera.h"

#include "Input.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.hpp>

DEFINE_LOG_CATEGORY(LogEditorCamera);

CEditorCamera::CEditorCamera(float fov, float aspectRatio, float nearClip, float farClip) {
    m_ActiveCamera = CreateRef<CSceneCamera>(fov, aspectRatio, nearClip, farClip);
}

void CEditorCamera::OnUpdate(float deltaTime, bool bUpdateInput) {
    if (SInput::IsKeyPressed(CZ_KEY(LeftAlt))) {
        const FVector2& mouse{ SInput::GetMouseX(), SInput::GetMouseY() };
        FVector2 delta         = (mouse - m_InitialMousePosition) * 0.003f;
        m_InitialMousePosition = mouse;

        if (!bUpdateInput) return;

        if (SInput::IsMouseButtonPressed(EMouseButton::Middle))
            MousePan(delta);
        else if (SInput::IsMouseButtonPressed(EMouseButton::Left))
            MouseRotate(delta);
        else if (SInput::IsMouseButtonPressed(EMouseButton::Right))
            MouseZoom(delta.y);
    }

    m_ActiveCamera->m_ViewDirty = true;
}

void CEditorCamera::OnEvent(IEvent& e) {
    FEventDispatcher dispatcher(e);
    dispatcher.Dispatch<FMouseScrolledEvent>(CZ_BIND_EVENT_FN(CEditorCamera::OnMouseScroll));
}

bool CEditorCamera::OnMouseScroll(FMouseScrolledEvent& e) {
    float delta = e.GetYOffset() * 0.1f;
    MouseZoom(delta);
    m_ActiveCamera->m_ViewDirty = true;
    // CZ_LOG(LogEditorCamera, Trace, e.ToString());

    return false;
}

void CEditorCamera::MousePan(const FVector2& delta) {
    auto [xSpeed, ySpeed] = PanSpeed();
    m_ActiveCamera->m_FocalPoint +=
        -m_ActiveCamera->GetRightDirection() * delta.x * xSpeed * m_ActiveCamera->m_Distance;
    m_ActiveCamera->m_FocalPoint +=
        m_ActiveCamera->GetUpDirection() * delta.y * ySpeed * m_ActiveCamera->m_Distance;
}

void CEditorCamera::MouseRotate(const FVector2& delta) {
    m_ActiveCamera->m_Rotation.x -= delta.y * RotationSpeed();
    m_ActiveCamera->m_Rotation.y -= delta.x * RotationSpeed();

    m_ActiveCamera->m_Rotation.x = std::clamp(m_ActiveCamera->m_Rotation.x, -89.0f, 89.0f);
}

void CEditorCamera::MouseZoom(float delta) {
    m_ActiveCamera->m_Distance -= delta * ZoomSpeed();
    if (m_ActiveCamera->m_Distance < 1.0f) {
        m_ActiveCamera->m_FocalPoint += m_ActiveCamera->GetForwardDirection();
        m_ActiveCamera->m_Distance = 1.0f;
    }
}

std::pair<float, float> CEditorCamera::PanSpeed() const {
    float x       = std::min(m_ActiveCamera->m_ViewportWidth / 1000.0f, 2.4f); // max = 2.4f
    float xFactor = 0.0366f * (x * x) - 0.1778f * x + 0.3021f;

    float y       = std::min(m_ActiveCamera->m_ViewportHeight / 1000.0f, 2.4f); // max = 2.4f
    float yFactor = 0.0366f * (y * y) - 0.1778f * y + 0.3021f;

    return { xFactor, yFactor };
}

float CEditorCamera::RotationSpeed() const { return 20.0f; }

float CEditorCamera::ZoomSpeed() const {
    float distance = m_ActiveCamera->m_Distance * 0.2f;
    distance       = std::max(distance, 0.0f);
    float speed    = distance * distance;
    speed          = std::min(speed, 100.0f); // max speed = 100
    return speed;
}

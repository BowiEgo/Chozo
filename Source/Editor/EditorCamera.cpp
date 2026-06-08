#include "EditorCamera.hpp"

#include <Core/Event/Input.hpp>
#include <Core/Event/KeyCodes.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.hpp>

DEFINE_LOG_CATEGORY(LogEditorCamera);

EditorCamera::EditorCamera(float fov, float aspectRatio, float nearClip, float farClip) {
    // m_ActiveCamera = CreateRef<CSceneCamera>(fov, aspectRatio, nearClip, farClip);
    m_ActiveCamera = SceneCamera::Create(fov, aspectRatio, nearClip, farClip);
}

void EditorCamera::OnUpdate(float deltaTime, bool bUpdateInput) {
    if (Input::IsKeyPressed(CZ_KEY(LeftAlt))) {
        const Vector2& mouse{ Input::GetMouseX(), Input::GetMouseY() };
        Vector2 delta          = (mouse - m_InitialMousePosition) * 0.003f;
        m_InitialMousePosition = mouse;

        if (!bUpdateInput) return;

        if (Input::IsMouseButtonPressed(MouseButton::Middle))
            MousePan(delta);
        else if (Input::IsMouseButtonPressed(MouseButton::Left))
            MouseRotate(delta);
        else if (Input::IsMouseButtonPressed(MouseButton::Right))
            MouseZoom(delta.y);
    }

    m_ActiveCamera->MarkViewDirty();
}

void EditorCamera::OnEvent(Event& e) {
    EventDispatcher dispatcher(e);
    dispatcher.Dispatch<MouseScrolledEvent>(CZ_BIND_FN(EditorCamera::OnMouseScroll));
}

bool EditorCamera::OnMouseScroll(MouseScrolledEvent& e) {
    float delta = e.GetYOffset() * 0.1f;
    MouseZoom(delta);
    m_ActiveCamera->MarkViewDirty();
    // CZ_LOG(LogEditorCamera, Trace, e.ToString());

    return false;
}

void EditorCamera::MousePan(const Vector2& delta) {
    auto [xSpeed, ySpeed] = PanSpeed();
    Vector3 translation =
        m_ActiveCamera->GetRightDirection() * -delta.x * xSpeed * m_ActiveCamera->GetDistance() +
        m_ActiveCamera->GetUpDirection() * delta.y * ySpeed * m_ActiveCamera->GetDistance();

    m_ActiveCamera->MoveFocalPoint(translation);
}

void EditorCamera::MouseRotate(const Vector2& delta) {
    m_ActiveCamera->MoveRotation({ -delta.y * RotationSpeed(), -delta.x * RotationSpeed() });
}

void EditorCamera::MouseZoom(float delta) {
    m_ActiveCamera->MoveDistance(-delta * ZoomSpeed());

    if (m_ActiveCamera->GetDistance() < 1.0f) {
        m_ActiveCamera->MoveFocalPoint(m_ActiveCamera->GetForwardDirection());
        m_ActiveCamera->SetDistance(1.0f);
    }
}

std::pair<float, float> EditorCamera::PanSpeed() const {
    float x       = std::min(m_ActiveCamera->GetViewportWidth() / 1000.0f, 2.4f); // max = 2.4f
    float xFactor = 0.0366f * (x * x) - 0.1778f * x + 0.3021f;

    float y       = std::min(m_ActiveCamera->GetViewportHeight() / 1000.0f, 2.4f); // max = 2.4f
    float yFactor = 0.0366f * (y * y) - 0.1778f * y + 0.3021f;

    return { xFactor, yFactor };
}

float EditorCamera::RotationSpeed() const { return 20.0f; }

float EditorCamera::ZoomSpeed() const {
    float distance = m_ActiveCamera->GetDistance() * 0.2f;
    distance       = std::max(distance, 0.0f);
    float speed    = distance * distance;
    speed          = std::min(speed, 100.0f); // max speed = 100
    return speed;
}

#include "CameraController.h"

namespace Chozo {
    void CameraController::Resize(float &width, float &height)
    {
        m_Camera.Resize(width, height);
    }

    void CameraController::Update(Timestep ts)
    {
        if (!m_IsActive) return;

        if (Input::IsKeyPressed(CZ_KEY_LEFT))
            m_CameraPosition.x -= m_CameraMoveSpeed * ts;
        else if (Input::IsKeyPressed(CZ_KEY_RIGHT))
            m_CameraPosition.x += m_CameraMoveSpeed * ts;

        if (Input::IsKeyPressed(CZ_KEY_DOWN))
            m_CameraPosition.y -= m_CameraMoveSpeed * ts;
        else if (Input::IsKeyPressed(CZ_KEY_UP))
            m_CameraPosition.y += m_CameraMoveSpeed * ts;

        if (Input::IsKeyPressed(CZ_KEY_A))
            m_CameraRotation += m_CameraRotationSpeed * ts;
        if (Input::IsKeyPressed(CZ_KEY_D))
            m_CameraRotation -= m_CameraRotationSpeed * ts;

        m_Camera.SetPosition(m_CameraPosition);
        m_Camera.SetRotation(m_CameraRotation);
    }

    bool CameraController::OnEvent(Event &e)
    {
        EventDispatcher dispatcher(e);
		dispatcher.Dispatch<MouseScrolledEvent>([this](MouseScrolledEvent& e) { return OnMouseScroll(e); });

        return true;
    }

    bool CameraController::OnMouseScroll(MouseScrolledEvent &e)
    {
        m_ZoomLevel -= e.GetYOffset() * 0.25f;
        m_ZoomLevel = std::max(m_ZoomLevel, 0.25f);

        m_Camera.Zoom(m_ZoomLevel);
        return true;
    }
}
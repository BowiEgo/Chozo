#include "Chozo.h"

namespace Chozo {

    class CameraController
    {
    public:
        CameraController()
            : m_Camera(1280.0f, 720.0f), m_CameraPosition(0.0f), m_CameraRotation(0.0f) {};
        ~CameraController() {};

        inline OrthographicCamera& GetCamera() { return m_Camera; }

        void SetActive(bool active) { m_IsActive = active; }

        void Resize(float& width, float& height);
        void Update(Timestep ts);

        bool OnEvent(Event& e);

    private:
        bool OnMouseScroll(MouseScrolledEvent& e);
    private:
        bool m_IsActive = true;

        OrthographicCamera m_Camera;
        glm::vec3 m_CameraPosition;
        float m_CameraRotation;

        float m_CameraMoveSpeed = 500.0f;
        float m_CameraRotationSpeed = 180.0f;
        float m_ZoomLevel = 1.0f;
        float m_LastScroll = 0.0f;
    };
}

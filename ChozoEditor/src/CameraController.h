#include "Chozo.h"

namespace Chozo {

    class CameraController
    {
    public:
        CameraController(OrthographicCamera* camera)
            : m_Camera(camera), m_CameraPosition(0.0f), m_CameraRotation(0.0f) {};
        ~CameraController() {};

        void Enable(bool enabled) { m_Enabled = enabled; }

        void Update(Timestep ts);

        bool OnEvent(Event& e);

    private:
        bool OnMouseScroll(MouseScrolledEvent& e);
    private:
        bool m_Enabled = true;

        OrthographicCamera* m_Camera;
        glm::vec3 m_CameraPosition;
        float m_CameraRotation;

        float m_CameraMoveSpeed = 500.0f;
        float m_CameraRotationSpeed = 180.0f;
        float m_ZoomLevel = 1.0f;
        float m_LastScroll = 0.0f;
    };
}

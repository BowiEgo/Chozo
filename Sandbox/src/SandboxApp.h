#define CHZO_ENTRY_POINT
#include "Chozo.h"

#include "Platform/OpenGL/OpenGLShader.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class ExampleLayer : public Chozo::Layer
{
public:
    ExampleLayer();

    virtual void OnAttach() override;
    virtual void OnDetach() override;
    virtual void OnUpdate(Chozo::Timestep ts) override;
    virtual void OnImGuiRender() override;
    virtual void OnEvent(Chozo::Event& event) override;
private:
    Chozo::Ref<Chozo::Framebuffer> m_Viewport_FBO;

    Chozo::Ref<Chozo::Shader> m_Shader;
    Chozo::Ref<Chozo::VertexArray> m_TriangleVA;

    Chozo::Ref<Chozo::Shader> m_flatColorShader;
    Chozo::Ref<Chozo::VertexArray> m_SquareVA;

    Chozo::Ref<Chozo::Texture2D> m_CheckerboardTexture, m_OpenGLLogoTexture;

    Chozo::OrthographicCamera m_Camera;
    glm::vec3 m_CameraPosition;
    float m_CameraMoveSpeed = 2.0f;
    float m_CameraRotation = 0.0f;
    float m_CameraRotationSpeed = 180.0f;

    glm::vec3 m_SquarePosition;
    float m_SquareMoveSpeed = 1.0f;

    glm::vec3 m_SquareColor = { 0.2f, 0.3f, 0.8f };
};

class Sandbox : public Chozo::Application
{
public:
    Sandbox()
        : Application("Sandbox")
    {
        CZ_INFO("Welcome to Chozo Sandbox!");
        PushLayer(new ExampleLayer());
    }

    ~Sandbox()
    {
    }
};

Chozo::Application *Chozo::CreateApplication()
{
    return new Sandbox();
};
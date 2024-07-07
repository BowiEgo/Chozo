#include "Parallax.h"

#include "Platform/OpenGL/OpenGLShader.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class ExampleLayer : public Parallax::Layer
{
public:
    ExampleLayer();

    virtual void OnAttach() override;
    virtual void OnDetach() override;
    virtual void OnUpdate(Parallax::Timestep ts) override;
    virtual void OnImGuiRender() override;
    virtual void OnEvent(Parallax::Event& event) override;
private:
    Parallax::Ref<Parallax::Framebuffer> m_Viewport_FBO;

    Parallax::ShaderLibrary m_ShaderLibrary;
    Parallax::Ref<Parallax::Shader> m_Shader;
    Parallax::Ref<Parallax::VertexArray> m_TriangleVA;

    Parallax::Ref<Parallax::Shader> m_flatColorShader;
    Parallax::Ref<Parallax::VertexArray> m_SquareVA;

    Parallax::Ref<Parallax::Texture2D> m_CheckerboardTexture, m_OpenGLLogoTexture;

    Parallax::OrthographicCamera m_Camera;
    glm::vec3 m_CameraPosition;
    float m_CameraMoveSpeed = 2.0f;
    float m_CameraRotation = 0.0f;
    float m_CameraRotationSpeed = 180.0f;

    glm::vec3 m_SquarePosition;
    float m_SquareMoveSpeed = 1.0f;

    glm::vec3 m_SquareColor = { 0.2f, 0.3f, 0.8f };
};

class Sandbox : public Parallax::Application
{
public:
    Sandbox()
    {
        PRX_INFO("Welcome to Parallax Sandbox!");
        PushLayer(new ExampleLayer());
    }

    ~Sandbox()
    {
    }
};

Parallax::Application *Parallax::CreateApplication()
{
    return new Sandbox();
};
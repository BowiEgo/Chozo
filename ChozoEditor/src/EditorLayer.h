#pragma once

#include "Chozo.h"

#include "Platform/OpenGL/OpenGLShader.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Chozo {

    class EditorLayer : public Chozo::Layer
    {
    public:
        EditorLayer();

        virtual void OnAttach() override;
        virtual void OnDetach() override;
        virtual void OnUpdate(Chozo::Timestep ts) override;
        virtual void OnImGuiRender() override;
        virtual void OnEvent(Chozo::Event& event) override;
    private:
        Ref<Chozo::Framebuffer> m_Viewport_FBO;

        Chozo::ShaderLibrary m_ShaderLibrary;
        Ref<Chozo::Shader> m_Shader;
        Ref<Chozo::Texture2D> m_CheckerboardTexture, m_OpenGLLogoTexture;

        Chozo::OrthographicCamera m_Camera;
        glm::vec3 m_CameraPosition;
        float m_CameraMoveSpeed = 2.0f;
        float m_CameraRotation = 0.0f;
        float m_CameraRotationSpeed = 180.0f;

        glm::vec3 m_SquareColor = { 0.2f, 0.3f, 0.8f };
    };
}


#pragma once

#include "Chozo.h"

#include "Platform/OpenGL/OpenGLShader.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "CameraController.h"

namespace Chozo {

    class EditorLayer : public Chozo::Layer
    {
    public:
        EditorLayer();

        virtual void OnAttach() override;
        virtual void OnDetach() override;
        virtual void OnUpdate(Chozo::Timestep ts) override;
        virtual void OnImGuiRender() override;
        virtual void OnEvent(Chozo::Event& e) override;
    private:
        Ref<Chozo::Framebuffer> m_Viewport_FBO;
        bool m_Viewport_Focused, m_Viewport_Hovered;

        Chozo::ShaderLibrary m_ShaderLibrary;
        Ref<Chozo::Shader> m_Shader;
        Ref<Chozo::Texture2D> m_CheckerboardTexture, m_OpenGLLogoTexture;

        Chozo::OrthographicCamera m_Camera;
        Scope<CameraController> m_CameraController;

        glm::vec3 m_SquareColor = { 1.0f, 1.0f, 1.0f };
    };
}


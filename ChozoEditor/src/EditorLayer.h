#pragma once

#include "Chozo.h"

#include "Platform/OpenGL/OpenGLShader.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "CameraController.h"

namespace Chozo {

    class EditorLayer : public Layer
    {
    public:
        EditorLayer();

        virtual void OnAttach() override;
        virtual void OnDetach() override;
        virtual void OnUpdate(Timestep ts) override;
        virtual void OnImGuiRender() override;
        virtual void OnEvent(Event& e) override;
    private:
        Ref<Framebuffer> m_Viewport_FBO;
        bool m_Viewport_Focused, m_Viewport_Hovered;

        ShaderLibrary m_ShaderLibrary;
        Ref<Shader> m_Shader;
        Ref<Texture2D> m_CheckerboardTexture, m_OpenGLLogoTexture;

        Scope<CameraController> m_CameraController;

        Ref<Scene> m_ActiveScene;

        glm::vec3 m_SquareColor = { 1.0f, 1.0f, 1.0f };
        entt::entity m_Squqre_Entity;
    };
}


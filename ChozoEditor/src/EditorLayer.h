#pragma once

#include "Chozo.h"

#include "Platform/OpenGL/OpenGLShader.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

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
        ImVec2 m_Viewport_Size;
        bool m_Viewport_Focused, m_Viewport_Hovered;

        ShaderLibrary m_ShaderLibrary;
        Ref<Shader> m_Shader;
        Ref<Texture2D> m_CheckerboardTexture, m_OpenGLLogoTexture;

        Ref<Scene> m_ActiveScene;

        Entity m_Square_Entity;
        glm::vec3 m_SquareColor = { 1.0f, 1.0f, 1.0f };

        Entity m_Camera_A, m_Camera_B;
        bool m_Camera_A_Is_Primary = true;
    };
}


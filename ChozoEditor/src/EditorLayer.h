#pragma once

#include "Chozo.h"

#include "Platform/OpenGL/OpenGLShader.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Panels/SceneHierarchyPanel.h"

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
        bool OnKeyPressed(KeyPressedEvent& e);
        bool OnMouseButtonPressed(MouseButtonPressedEvent& e);

        std::pair<float, float> GetMouseViewportSpace();
        
        void NewScene();
        void OpenScene();
        void SaveSceneAs();
    private:
        Ref<Framebuffer> m_Viewport_FBO, m_ID_FBO;
        ImVec2 m_ViewportSize;
		glm::vec2 m_ViewportBounds[2];
        
        bool m_ViewportFocused, m_ViewportHovered;

        ShaderLibrary m_ShaderLibrary;
        Ref<Shader> m_Shader;
        Ref<Texture2D> m_CheckerboardTexture, m_OpenGLLogoTexture;

        Ref<Scene> m_ActiveScene;

        Entity m_Square_Entity;
        glm::vec3 m_SquareColor = { 1.0f, 1.0f, 1.0f };

        EditorCamera m_EditorCamera;

        Entity m_Camera_A, m_Camera_B;
        bool m_Camera_A_Is_Primary = true;
        bool m_AllowViewportCameraEvents;

        // Panels
        SceneHierarchyPanel m_SceneHierarchyPanel;

        // Gizmo
        int m_GizmoType = -1; // -1 = no gizmo
		int m_GizmoMode = 0; // 0 = local

        Entity m_Entity_Hovered;
    };
}


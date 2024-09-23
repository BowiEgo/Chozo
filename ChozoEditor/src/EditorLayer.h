#pragma once

#include "Chozo.h"

#include "Platform/OpenGL/OpenGLShader.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <ImGuizmo.h>

#include "Panels/SceneHierarchyPanel.h"
#include "Panels/ContentBrowserPanel.h"
#include "Panels/EnvironmentPanel.h"
#include "Panels/TextureViewerPanel.h"

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
        Entity PickEntity(uint32_t mx, uint32_t my);

        void OnDragAndDrop();

        bool OnKeyPressed(KeyPressedEvent& e);
        bool OnMouseButtonPressed(MouseButtonPressedEvent& e);
        bool OnMouseButtonReleased(MouseButtonReleasedEvent& e);

        std::pair<float, float> GetMouseViewportSpace();
        
        void NewScene();
        void OpenScene();
        void OpenScene(const std::filesystem::path& path);
        void SaveSceneAs();

        void OnScenePlay();
        void OnSceneStop();
        // UI panels
        void RenderUI_Toolbar();
    private:
        Ref<Scene> m_ActiveScene;
        std::string m_SceneFileName;
		Ref<SceneRenderer> m_ViewportRenderer;
        ImVec2 m_ViewportSize;
		glm::vec2 m_ViewportBounds[2];
        bool m_ViewportFocused, m_ViewportHovered;

        EditorCamera m_EditorCamera;

        bool m_AllowViewportCameraEvents;

        // Panels
        SceneHierarchyPanel m_SceneHierarchyPanel;
        ContentBrowserPanel m_ContentBrowserPanel;
        EnvironmentPanel m_EnvironmentPanel;
        TextureViewerPanel m_TextureViewerPanel;

        // Gizmo
        int m_GizmoType = ImGuizmo::OPERATION::TRANSLATE; // -1 = no gizmo
		int m_GizmoMode = 0; // 0 = local

        Entity m_Entity_Selected;

        enum class SceneState
        {
            Edit = 0, Play
        };

        SceneState m_SceneState = SceneState::Edit;

        Ref<Texture2D> m_IconPlay, m_IconStop;

    };
}


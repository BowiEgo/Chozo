#pragma once

#include "Chozo.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <ImGuizmo.h>

#include "Panels/SceneHierarchyPanel.h"
#include "Panels/PropertiesPanel.h"
#include "Panels/ContentBrowser/ContentBrowserPanel.h"
#include "Panels/MaterialPanel.h"
#include "Panels/EnvironmentPanel.h"
#include "Panels/TextureViewerPanel.h"

namespace Chozo {

    class EditorLayer : public Layer
    {
    public:
        EditorLayer();

        void OnAttach() override;
        void OnDetach() override;
        void OnUpdate(TimeStep ts) override;
        void OnImGuiRender() override;
        void OnEvent(Event& e) override;
    private:
        Entity PickEntity(float mx, float my);

        void OnDragAndDrop(AssetHandle handle);

        bool OnKeyPressed(KeyPressedEvent& e);
        bool OnMouseButtonPressed(MouseButtonPressedEvent& e);
        bool OnMouseButtonReleased(MouseButtonReleasedEvent& e);

        std::pair<float, float> GetMouseViewportSpace() const;
        
        void NewProject();
        void OpenProject();
        void OpenProject(const fs::path& path);
        void SaveProjectAs();

        void OnScenePlay();
        void OnSceneStop();
        // UI panels
        void RenderUI_Toolbar();
    private:
        Ref<Scene> m_ActiveScene;
        std::string m_SceneFileName;
		Ref<SceneRenderer> m_ViewportRenderer;
        ImVec2 m_ViewportSize;
		glm::vec2 m_ViewportBounds[2]{};
        bool m_ViewportFocused{}, m_ViewportHovered{};

        EditorCamera m_EditorCamera{};

        bool m_AllowViewportCameraEvents{};

        // Panels
        SceneHierarchyPanel m_SceneHierarchyPanel;
        PropertiesPanel m_PropertiesPanel;
        ContentBrowserPanel m_ContentBrowserPanel;
        EnvironmentPanel m_EnvironmentPanel;
        MaterialPanel m_MaterialPanel;
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


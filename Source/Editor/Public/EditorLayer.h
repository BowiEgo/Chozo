#pragma once

#include "Core.h"
#include "FileDialog.h"
#include "IconManager.h"
#include "Layer.h"
#include "Overlay.h"
#include "Renderer.h"
#include "Scene.h"

#include "EditorNode.h"
#include "EditorNodeTree.h"
#include "SyncLayer.h"

#include "AssetsPanel.h"
#include "ConsolePanel.h"
#include "ContentBrowserPanel.h"
#include "MaterialPanel.h"
#include "PropertiesPanel.h"
#include "SceneHierarchyPanel.h"
#include "TextureViewerPanel.h"

#include "EditorCamera.h"

DECLARE_LOG_CATEGORY_EXTERN(LogEditorLayer, Info);

class EditorLayer : public ILayer {
public:
    EditorLayer();

    void OnAttach() override;
    void OnDetach() override;
    void OnUpdate(float deltaTime) override;
    void OnImGuiRender() override;
    void OnEvent(IEvent& e) override;
    bool OnKeyPressed(FKeyPressedEvent& e) override;
    // bool OnMouseButtonPressed(FMouseButtonPressedEvent& e) override;
    // bool OnMouseButtonReleased(FMouseButtonReleasedEvent& e) override;

private:
    void NewProject();
    void OpenProject();
    void OpenProject(const std::filesystem::path& path);
    void SaveProjectAs();

private:
    CViewport* m_Viewport;
    CRenderer* m_ViewportRenderer;
    ImVec2 m_ViewportSize{ 1080, 720 };
    bool m_ViewportFocused{}, m_ViewportHovered{};

    bool m_VSyncEnabled = true;

    CEditorCamera m_EditorCamera;

    EPolygonMode m_PolygonMode = EPolygonMode::Fill;

    TScope<FScene> m_Scene;
    CEditorNodeTree m_NodeTree;
    TScope<FSyncLayer> m_SyncLayer;

    // UI
    UOverlay m_Overlay;
    bool m_IsOverlayOpen = true;

    // Panels
    ConsolePanel m_ConsolePanel;
    SceneHierarchyPanel m_SceneHierarchyPanel;
    PropertiesPanel m_PropertiesPanel;
    ContentBrowserPanel m_ContentBrowserPanel;
    MaterialPanel m_MaterialPanel;
    TextureViewerPanel m_TextureViewerPanel;
    AssetsPanel m_AssetsPanel;
};

extern "C" {
EDITOR_API ILayer* CreateEditorLayer() { return new EditorLayer(); }
}

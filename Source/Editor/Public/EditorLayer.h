#pragma once

#include "Core.h"
#include "Layer.h"
#include "Overlay.h"
#include "Renderer.h"

#include "AssetsPanel.h"
#include "ConsolePanel.h"
#include "ContentBrowserPanel.h"
#include "EditorExport.h"
#include "MaterialPanel.h"
#include "PropertiesPanel.h"
#include "SceneHierarchyPanel.h"
#include "TextureViewerPanel.h"

DECLARE_LOG_CATEGORY_EXTERN(LogEditorLayer, Info);

class EditorLayer : public ILayer {
public:
    EditorLayer();

    void OnAttach() override;
    void OnDetach() override;
    void OnUpdate(FTimeStep ts) override;
    void OnImGuiRender() override;
    void OnEvent(IEvent& e) override;

private:
    void NewProject();
    void OpenProject();
    void OpenProject(const std::filesystem::path& path);
    void SaveProjectAs();

private:
    CRenderer* m_ViewportRenderer;
    ImVec2 m_ViewportSize;

    bool m_VSyncEnabled = true;

    Overlay m_Overlay;
    bool m_IsOverlayOpen = true;

    // Panels
    ConsolePanel m_ConsolePanel;
    bool m_IsConsoleOpen = true;

    SceneHierarchyPanel m_SceneHierarchyPanel;
    bool m_IsSceneHierarchyOpen = true;

    PropertiesPanel m_PropertiesPanel;
    bool m_IsPropertiesOpen = true;

    ContentBrowserPanel m_ContentBrowserPanel;
    bool m_IsContentBrowserOpen = true;

    MaterialPanel m_MaterialPanel;
    bool m_IsMaterialOpen = true;

    TextureViewerPanel m_TextureViewerPanel;
    bool m_IsTextureViewerOpen = true;

    AssetsPanel m_AssetsPanel;
    bool m_IsAssetsOpen = true;
};

extern "C" {
EDITOR_API ILayer* CreateEditorLayer() { return new EditorLayer(); }
}

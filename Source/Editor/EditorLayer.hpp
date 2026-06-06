#pragma once

#include <Core/Layer/Layer.hpp>
#include <Runtime/RenderCore/Renderer.hpp>
#include <Runtime/RenderCore/Viewport.hpp>

#include "Renderer/Vulkan/VulkanImGuiRenderer.hpp"
#include "SyncBridge.hpp"

#include "Panels/AssetsPanel.hpp"
#include "Panels/ConsolePanel.hpp"
// #include "Panels/ContentBrowserPanel.hpp"
// #include "Panels/MaterialPanel.hpp"
#include "Panels/PropertiesPanel.hpp"
#include "Panels/SceneHierarchyPanel.hpp"
#include "Panels/TextureViewerPanel.hpp"

using namespace CZ;

DECLARE_LOG_CATEGORY_EXTERN(LogEditorLayer, Info);

class EditorLayer : public Layer {
public:
    EditorLayer();
    ~EditorLayer() override;

public:
    void OnAttach() override;

    void OnDetach() override;

    void OnUpdate(float deltaTime) override;

    void OnRender() override;

    void OnEvent(Event& e) override;

    bool OnKeyPressed(KeyPressedEvent& e) override;

    void Draw(CommandList cmdList);

    VulkanImGuiRenderer* GetImGuiRenderer() const { return m_ImGuiRenderer.get(); }

private:
    void Init();

    void SetFont(std::string font);

    void SetDarkThemeColors();

    void NewProject();
    void OpenProject();
    void OpenProject(const std::filesystem::path& path);
    void SaveProjectAs();

    bool m_BlockEvents;

    ImVec2 m_ViewportSize{ 1080, 720 };

    bool m_ViewportFocused{}, m_ViewportHovered{};

    Scope<VulkanImGuiRenderer> m_ImGuiRenderer;

    Renderer m_ViewportRenderer;
    Scene m_Scene;
    Viewport m_Viewport;

    EditorNodeTree m_NodeTree;
    Scope<SyncBridge> m_SyncBridge;

    // Panels
    ConsolePanel m_ConsolePanel;
    SceneHierarchyPanel m_SceneHierarchyPanel;
    PropertiesPanel m_PropertiesPanel;
    // ContentBrowserPanel m_ContentBrowserPanel;
    // MaterialPanel m_MaterialPanel;
    TextureViewerPanel m_TextureViewerPanel;
    AssetsPanel m_AssetsPanel;
};
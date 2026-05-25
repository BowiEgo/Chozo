#pragma once

#include <Core/Layer/Layer.hpp>
#include <Runtime/RenderCore/Renderer.hpp>
#include <Runtime/RenderCore/Viewport.hpp>

#include "Renderer/Vulkan/VulkanImGuiRenderer.hpp"

#include "Panels/AssetsPanel.h"
#include "Panels/ConsolePanel.h"
// #include "Panels/ContentBrowserPanel.h"
// #include "Panels/MaterialPanel.h"
// #include "Panels/PropertiesPanel.h"
// #include "Panels/SceneHierarchyPanel.h"
#include "Panels/TextureViewerPanel.h"

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
    Viewport m_Viewport;

    // Panels
    ConsolePanel m_ConsolePanel;
    // SceneHierarchyPanel m_SceneHierarchyPanel;
    // PropertiesPanel m_PropertiesPanel;
    // ContentBrowserPanel m_ContentBrowserPanel;
    // MaterialPanel m_MaterialPanel;
    TextureViewerPanel m_TextureViewerPanel;
    AssetsPanel m_AssetsPanel;
};
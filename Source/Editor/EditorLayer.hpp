#pragma once
#include <filesystem>
#include <string>

#include <Core/Layer/Layer.hpp>
#include <Runtime/RenderCore/Renderer.hpp>
#include <Runtime/RenderCore/Viewport.hpp>

#include "EditorCamera.hpp"
#include "SyncBridge.hpp"
#include <Runtime/UI/UIRenderBackend.hpp>

#include "Panels/ConsolePanel.hpp"
// #include "Panels/ContentBrowserPanel.hpp"
// #include "Panels/MaterialPanel.hpp"
#include "Panels/PropertiesPanel.hpp"
#include "Panels/SceneHierarchyPanel.hpp"

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

    UIRenderBackendObj* GetImGuiRenderer() const { return m_ImGuiRenderer.get(); }

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

    UIRenderBackendPtr m_ImGuiRenderer;

    Renderer m_ViewportRenderer;
    Scene m_Scene; // view; owned by the viewport

    // Stable id of the texture currently registered with ImGui for the viewport image.
    UUID m_ViewportTextureID = UUID::Invalid();
    Viewport m_Viewport;
    EditorCamera m_EditorCamera;

    EditorNodeTree m_NodeTree;
    Scope<SyncBridge> m_SyncBridge;

    // Panels
    ConsolePanel m_ConsolePanel;
    SceneHierarchyPanel m_SceneHierarchyPanel;
    PropertiesPanel m_PropertiesPanel;
};
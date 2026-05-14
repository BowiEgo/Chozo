#pragma once

#include <Core/Layer/Layer.h>

#include "Renderer/Vulkan/VulkanImGuiRenderer.h"

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

    bool m_BlockEvents;

    ImVec2 m_ViewportSize{ 1080, 720 };

    bool m_ViewportFocused{}, m_ViewportHovered{};

    Scope<VulkanImGuiRenderer> m_ImGuiRenderer;
};
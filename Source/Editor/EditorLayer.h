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

    void Draw();

private:
    void Init();

    void SetFont(std::string font);

    void SetDarkThemeColors();

    bool m_BlockEvents;

    Scope<VulkanImGuiRenderer> m_ImGuiRenderer;
};
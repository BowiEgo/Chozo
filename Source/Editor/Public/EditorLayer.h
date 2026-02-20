#pragma once

#include "Chozo.h"

#include "imgui.h"

class EditorLayer : public ILayer {
public:
    EditorLayer();

    void OnAttach() override;
    void OnDetach() override;
    void OnUpdate(FTimeStep ts) override;
    void OnImGuiRender() override;
    void OnEvent(IEvent& e) override;

private:
    CRenderer* m_ViewportRenderer;
    ImVec2 m_ViewportSize;
};
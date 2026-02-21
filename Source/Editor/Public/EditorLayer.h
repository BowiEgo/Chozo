#pragma once

#include "Chozo.h"
#include "Overlay.h"

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
};
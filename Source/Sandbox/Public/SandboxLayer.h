#pragma once

#include "Core.h"
#include "Layer.h"
#include "Overlay.h"
#include "Renderer.h"

#include "SandboxExport.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSandboxLayer, Info);

class SandboxLayer : public ILayer {
public:
    SandboxLayer();

    void OnAttach() override;
    void OnDetach() override;
    void OnUpdate(float deltaTime) override;
    void OnImGuiRender() override;
    void OnEvent(IEvent& e) override;

private:
    void NewProject();
    void OpenProject();
    void OpenProject(const std::filesystem::path& path);
    void SaveProjectAs();

private:
    CViewport* m_Viewport;
    CRenderer* m_ViewportRenderer;
    ImVec2 m_ViewportSize{ 1080, 720 };

    bool m_VSyncEnabled = true;

    UOverlay m_Overlay;
    bool m_IsOverlayOpen = true;
};

extern "C" {
SANDBOX_API ILayer* CreateSandboxLayer() { return new SandboxLayer(); }
}

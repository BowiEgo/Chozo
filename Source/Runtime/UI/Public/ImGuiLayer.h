#pragma once

#include "Event.h"
#include "ImGuiRenderer.h"
#include "Layer.h"
#include "Module.h"
#include "Renderer.h"
#include "Scope.h"
#include "UIExport.h"

DECLARE_LOG_CATEGORY_EXTERN(LogImGuiLayer, Info);

class UI_API CImGuiLayer : public ILayer {
public:
    CImGuiLayer(CWindow* window, CRenderer* renderer);
    ~CImGuiLayer();

private:
    TScope<IImGuiRenderer> CreateRenderer(CWindow* window, CGraphicsContext* context);

public:
    virtual void OnAttach() override;
    virtual void OnDetach() override;
    virtual void OnImGuiRender() override;
    virtual void OnEvent(IEvent& e) override;

    void BlockEvents(bool block) { m_BlockEvents = block; }
    void Begin();
    void End();
    void SetFont(std::string fontPath, float dpi);
    void SetDarkThemeColors();

    void Render(const TRef<IRHICommandBuffer> cmdBuffer);

private:
    CModule m_RHIModule;

    CWindow* m_Window;
    CRenderer* m_Renderer;

    bool m_BlockEvents = false;
    TScope<IImGuiRenderer> m_ImGuiRenderer;
};
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
    CImGuiLayer(CWindow* window, IRHIContext* rhiContext);
    ~CImGuiLayer();

private:
    TScope<IImGuiRenderer> CreateRenderer(CWindow* window, IRHIContext* rhiContext);

public:
    virtual void OnAttach() override;
    virtual void OnDetach() override;
    virtual void OnEvent(IEvent& e) override;

    void BlockEvents(bool block) { m_BlockEvents = block; }
    void Begin();
    void Render(const std::function<void()>& renderCb);
    void End();
    void SetFont(std::string font);
    void SetDarkThemeColors();

    void Draw(const TRef<IRHICommandList>& cmdBuffer);

private:
    CModule m_RHIModule;

    CWindow* m_Window;
    IRHIContext* m_Context;

    bool m_BlockEvents = false;
    TScope<IImGuiRenderer> m_ImGuiRenderer;
};
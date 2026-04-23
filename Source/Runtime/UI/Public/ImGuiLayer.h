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
    static CImGuiLayer& Get() {
        static CImGuiLayer instance;
        return instance;
    }

    CImGuiLayer(const CImGuiLayer&)            = delete;
    CImGuiLayer& operator=(const CImGuiLayer&) = delete;

private:
    CImGuiLayer();
    ~CImGuiLayer() = default;

    TScope<IImGuiRenderer> CreateRenderer(CWindow* window, IRHIContext* rhiContext);

public:
    virtual void OnAttach() override;
    virtual void OnDetach() override;
    virtual void OnEvent(IEvent& e) override;

    void Init(CWindow* window, IRHIContext* rhiContext);
    void BlockEvents(bool block) { m_BlockEvents = block; }
    void Begin();
    void Render(const std::function<void()>& renderCb);
    void End();
    void SetFont(std::string font);
    void SetDarkThemeColors();

    void Draw(const TRef<IRHICommandList>& cmdBuffer);

    IImGuiRenderer* GetRenderer() { return m_ImGuiRenderer.get(); }

private:
    CModule m_RHIModule;

    CWindow* m_Window;
    IRHIContext* m_Context;

    bool m_BlockEvents = false;
    TScope<IImGuiRenderer> m_ImGuiRenderer;
};
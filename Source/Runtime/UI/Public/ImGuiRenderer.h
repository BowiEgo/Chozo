#pragma once

#include "CoreMinimal.h"
#include "GraphicsContext.h"
#include "RHICommandBuffer.h"
#include "UIExport.h"
#include "Window.h"

#include "imgui.h"

DECLARE_LOG_CATEGORY_EXTERN(LogImGuiRenderer, Info);

class UI_API IImGuiRenderer {
public:
    IImGuiRenderer(CWindow* window, CGraphicsContext* context);
    virtual ~IImGuiRenderer();

    virtual void Init(ImGuiContext* ctx) = 0;
    virtual void Shutdown() = 0;
    virtual void NewFrame() = 0;
    virtual void Render(ImDrawData* drawData, const TRef<IRHICommandBuffer> cmdBuffer) = 0;

protected:
    CWindow* m_Window;
    CGraphicsContext* m_Context;
};
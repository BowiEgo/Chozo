#pragma once

#include "CoreMinimal.h"
#include "GraphicsContext.h"
#include "Module.h"
#include "RenderCoreExport.h"
#include "RendererWindow.h"
#include "Scope.h"

DECLARE_LOG_CATEGORY_EXTERN(LogRenderer, Info);

class RENDER_CORE_API CRenderer {
    static const int MAX_FRAMES_IN_FLIGHT = 2;
    uint32_t m_CurrentFrame = 0;

public:
    CRenderer(IRendererWindow* windowHandle);
    ~CRenderer();

    void Init();
    void Tick();
    void Shutdown();

    IGraphicsContext* GetGraphicsContext() const { return m_Context.get(); }

private:
    IRendererWindow* m_Window;
    TScope<IGraphicsContext> m_Context;
    CModule m_RHIModule;
    TRef<IRHISyncObject> m_SyncObjects[MAX_FRAMES_IN_FLIGHT];
    TRef<IRHICommandBuffer> m_CommandBuffers[MAX_FRAMES_IN_FLIGHT];
};

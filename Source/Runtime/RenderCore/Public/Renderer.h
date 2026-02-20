#pragma once

#include "CoreMinimal.h"
#include "GraphicsContext.h"
#include "Module.h"
#include "RenderCoreExport.h"
#include "RendererWindow.h"
#include "Scope.h"

DECLARE_LOG_CATEGORY_EXTERN(LogRenderer, Info);

using FOnRenderUI = std::function<void(const TRef<IRHICommandBuffer>&)>;

class RENDER_CORE_API CRenderer {
    static const int MAX_FRAMES_IN_FLIGHT = 3;

public:
    CRenderer(IRendererWindow* windowHandle);
    ~CRenderer();

    void Init();
    void Tick();
    void Shutdown();

    void SetUICallback(FOnRenderUI callback) { m_UICallback = std::move(callback); }

    CGraphicsContext* GetGraphicsContext() const { return m_Context.get(); }
    TRef<IRHICommandBuffer> GetCommandBuffer() const { return m_CommandBuffers[m_CurrentFrame]; }
    TRef<IRHIFrameBuffer> GetSceneFrameBuffer() const { return m_SceneFrameBuffer; }

private:
    CModule m_RHIModule;

    IRendererWindow* m_Window;
    TScope<CGraphicsContext> m_Context;

    TRef<IRHISyncObject> m_SyncObjects[MAX_FRAMES_IN_FLIGHT];
    TRef<IRHICommandBuffer> m_CommandBuffers[MAX_FRAMES_IN_FLIGHT];
    uint32_t m_CurrentFrame = 0;

    TRef<IRHIPipeline> m_ScenePipeline;

    FOnRenderUI m_UICallback = nullptr;

    TRef<IRHIFrameBuffer> m_SceneFrameBuffer;
};

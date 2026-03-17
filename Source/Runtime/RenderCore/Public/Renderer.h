#pragma once

#include "CoreMinimal.h"
#include "Module.h"
#include "RHICommandList.h"
#include "RHIContext.h"
#include "RendererWindow.h"
#include "Scope.h"
#include "Viewport.h"

#include "RenderCoreExport.h"

DECLARE_LOG_CATEGORY_EXTERN(LogRenderer, Info);

struct FFrameResource {
    TRef<IRHICommandPool> CommandPool;
    TRef<IRHICommandList> CommandBuffer;
    TRef<IRHISyncObject> RenderFence;
};

using FOnRenderUI = std::function<void(const TRef<IRHICommandList>&)>;

class RENDER_CORE_API CRenderer {

public:
    CRenderer(IRendererWindow* windowHandle);
    ~CRenderer();

    void Init();
    void Tick(float deltaTime);
    void Shutdown();
    CViewport* CreateViewport(const std::string name, uint32 width, uint32 height);

    void SetUICallback(FOnRenderUI callback) { m_UICallback = std::move(callback); }
    void SetPresentMode(const EPresentMode mode) {
        m_GraphicContext->GetSwapchain()->SetPresentMode(mode);
    }
    void RecreateSwapchain(const FExtent2D& frameBufferSize) {
        m_GraphicContext->GetSwapchain()->Recreate(frameBufferSize);
    }

    IRendererWindow* GetWindow() const { return m_Window; }
    IRHIContext* GetGraphicContext() const { return m_GraphicContext.get(); }
    TRef<IRHICommandList> GetCommandBuffer() const {
        return m_Frames[m_CurrentFrameIndex].CommandBuffer;
    }

private:
    CModule m_RHIModule;

    IRendererWindow* m_Window;
    TScope<IRHIContext> m_GraphicContext;

    std::vector<FFrameResource> m_Frames;
    uint32 m_CurrentFrameIndex = 0;

    std::vector<TScope<CViewport>> m_Viewports;

    TRef<IRHIPipeline> m_ScenePipeline;

    FOnRenderUI m_UICallback = nullptr;
};

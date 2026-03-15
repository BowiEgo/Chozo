#pragma once

#include "CameraUniformManager.h"
#include "CoreMinimal.h"
#include "Module.h"
#include "RHICommandList.h"
#include "RHIContext.h"
#include "RHIFrameBuffer.h"
#include "RenderCoreExport.h"
#include "RendererWindow.h"
#include "Scope.h"

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
    TRef<IRHIFrameBuffer> GetSceneFrameBuffer() const { return m_SceneFrameBuffer; }

private:
    CModule m_RHIModule;

    IRendererWindow* m_Window;
    TScope<IRHIContext> m_GraphicContext;

    std::vector<FFrameResource> m_Frames;
    uint32_t m_CurrentFrameIndex = 0;

    TRef<IRHIPipeline> m_ScenePipeline;

    FOnRenderUI m_UICallback = nullptr;

    TRef<IRHIFrameBuffer> m_SceneFrameBuffer;

    CCamera m_Camera;
    TScope<CCameraUniformManager> m_CameraUniformManager;
};

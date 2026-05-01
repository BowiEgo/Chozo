#pragma once

#include "CoreMinimal.h"
#include "Cube.h"
#include "LightRegister.h"
#include "Material.h" // TODO: Remove
#include "MeshManager.h"
#include "Module.h"
#include "RHICommandList.h"
#include "RHIContext.h"
#include "RendererWindow.h"
#include "Scope.h"
#include "Sphere.h"
#include "Texture.h"
#include "Timer.h"
#include "Viewport.h"

#include "RenderCoreExport.h"

#include "Cube.h"
#include "Quad.h"

DECLARE_LOG_CATEGORY_EXTERN(LogRenderer, Info);

#define RENDERER_PROFILE_SLOTS                                                                     \
    X(TotalFrame, "Total FrameTime")                                                               \
    X(CubemapSampler, "EquirectToCubemap Pass")                                                    \
    X(Skybox, "Skybox Pass")                                                                       \
    X(Composite, "Composite Pass")                                                                 \
    X(ImGUI, "ImGUI Pass")                                                                         \
    X(GraphCompile, "Graph Compile")                                                               \
    X(GraphExecute, "Graph Execute")

#define X(name, displayName) name,
enum class ERendererProfileSlot : uint32_t { RENDERER_PROFILE_SLOTS COUNT };
#undef X

#define X(name, displayName) displayName,
const char* const GRendererProfileSlotNames[] = { RENDERER_PROFILE_SLOTS };
#undef X

struct FFrameResource {
    TRef<IRHICommandPool> CommandPool;
    TRef<IRHICommandList> CommandList;
    TRef<IRHISyncObject> RenderFence;
};

using FOnRenderUI = std::function<void(const TRef<IRHICommandList>&)>;

class RENDER_CORE_API CRenderer {

public:
    CRenderer(IRendererWindow* windowHandle);
    ~CRenderer();

    void Init();
    void Tick(float deltaTime);
    void Clear();
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
    TRef<IRHICommandList> GetCommandList() const {
        return m_Frames[m_CurrentFrameIndex].CommandList;
    }
    PerformanceProfiler* GetPerformanceProfiler() { return m_Profiler.get(); }

    TRef<CMaterial> GetMaterial() { return m_GBufferMat; } // TODO: Remove
    void SetDebugMode(int mode) { m_DebugMode = mode; }    // TODO: Remove
    const int GetDebugMode() { return m_DebugMode; }       // TODO: Remove

private:
    CModule m_RHIModule;
    TScope<PerformanceProfiler> m_Profiler;

    IRendererWindow* m_Window;
    TScope<IRHIContext> m_GraphicContext;

    std::vector<FFrameResource> m_Frames;
    uint32 m_CurrentFrameIndex = 0;

    std::vector<TScope<CViewport>> m_Viewports;

    TRef<IRHIPipeline> m_CubemapSamplerPipeline, m_IrradiancePipeline, m_PrefilteredPipeline,
        m_SkyboxPipeline, m_DebugPipeline, m_PBRPipeline;        // TODO: Remove
    TRef<IRHIBuffer> m_DebugUniformBuffer, m_SceneUniformBuffer; // TODO: Remove

    TRef<IRHIBuffer> m_CubemapCameraBuffer; // TODO: Remove

    TRef<CTexture> m_SkyboxTex, m_SkyboxCubemap, m_BRDFLutTex, m_SkyboxIrradianceCubemap,
        m_SkyboxPrefilteredCubemap; // TODO: Remove
    TRef<FCube> m_Cube;
    TRef<FQuad> m_Quad;

    TRef<CMaterial> m_SolidMat, m_GBufferMat, m_PBRMat; // TODO: Remove

    int m_DebugMode = 0; // TODO: Remove

    FOnRenderUI m_UICallback = nullptr;

    bool m_bEnvSampled = false;
};

#if 1
    // Create a unique timer variable named e.g., timer123
    #define CZ_RENDERER_SCOPE_PERF(slot)                                                           \
        ScopePerfTimer CZ_CONCAT(timer, __LINE__)(static_cast<uint32_t>(slot),                     \
                                                  this -> GetPerformanceProfiler())

    #define CZ_RENDERER_SCOPE_TIMER(name) ScopedTimer CZ_CONCAT(timer, __LINE__)(name);
#else
    #define CZ_RENDERER_SCOPE_PERF(slot)
    #define CZ_RENDERER_SCOPE_TIMER(name)
#endif
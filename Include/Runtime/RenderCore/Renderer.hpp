#pragma once

#include <Core/Header/Types.h>
#include <Runtime/App/StartupHost.hpp>
#include <Runtime/RHI/GraphicsContext.hpp>
#include <Runtime/RenderCore/Viewport.hpp>
#include <Runtime/Window/Window.hpp>

namespace CZ {

struct RendererSpecification {
    Window Window;
};

struct FrameResource {
    CommandPool CommandPool;
    CommandList CommandList;
};

struct RendererObj {
    uint32 CurrentFrameIndex = 0;
    Window Window;
    DrawFunc FinalPassDrawFunc;

    std::vector<FrameResource> Frames;

    std::vector<Viewport> Viewports;
};

struct Renderer : Handle<struct RendererObj> {
    static Renderer Create(const RendererSpecification& spec);

    void Shutdown();
    void Tick(float deltaTime);
    void SetDrawFuncToFinalPass(const DrawFunc& func);

    Viewport CreateViewport(const std::string name, uint32 width, uint32 height);
    std::vector<Viewport> GetViewports();
};

} // namespace CZ

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
#pragma once

#include "Runtime/App/StartupHost.h"
#include <Core/Header/Types.h>
#include <Runtime/RHI/GraphicsContext.h>
#include <Runtime/Window/Window.h>

namespace CZ {

struct RendererSpecification {
    Window Window;
};

struct RendererObj;

struct Renderer : Handle<struct RendererObj> {
    static Renderer Create(const RendererSpecification& spec);

    void Init();
    void Shutdown();
    void Tick(float deltaTime);

    void SetDrawFuncToFinalPass(const DrawFunc& func);
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
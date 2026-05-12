#pragma once

#include <Core/Header/Types.h>
#include <Runtime/RHI/GraphicsContext.h>
#include <Runtime/Window/Window.h>

namespace CZ {

struct RendererSpecification {
    Window Window;
    GraphicsContext GraphicsContext;
};

struct RendererObj;

struct Renderer : Handle<RendererObj> {
    static Renderer Create(const RendererSpecification& spec);
    static void Destroy(Renderer renderer);

    void Init();
    void Tick(float deltaTime);
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
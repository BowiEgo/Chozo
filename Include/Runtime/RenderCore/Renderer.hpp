#pragma once
#include <string>
#include <vector>

#include <Core/Header/Types.h>
#include <Runtime/App/StartupHost.hpp>
#include <Runtime/RHI/GraphicsContext.hpp>
#include <Runtime/RenderCore/ShaderRegistry.hpp>
#include <Runtime/RenderCore/Viewport.hpp>
#include <Runtime/Window/Window.hpp>

namespace CZ {

struct RendererSpecification {
    Window Window;

    // Registries the renderer hands to the scenes it creates viewports for. Non-owning; the
    // engine owns them and they outlive the renderer.
    MeshRegistry* MeshRegistry     = nullptr;
    ShaderRegistry* ShaderRegistry = nullptr;
};

struct FrameResource {
    Scope<CommandPoolObj> Pool;
    Scope<CommandListObj> List;
};

struct RendererObj {
    uint32 CurrentFrameIndex = 0;
    Window Window;
    DrawFunc FinalPassDrawFunc;
    MeshRegistry* MeshRegistry = nullptr;

    // Owned resources; the renderer hands out views.
    std::vector<FrameResource> Frames;
    std::vector<Scope<ViewportObj>> Viewports;
    Scope<PipelineObj> TestPipeline;
};

struct Renderer : Handle<struct RendererObj> {
    using Handle<struct RendererObj>::Handle;

    /// Creates a renderer owned by the caller.
    static Scope<RendererObj> Create(const RendererSpecification& spec);

    void Shutdown();
    void Tick(float deltaTime);
    void SetDrawFuncToFinalPass(const DrawFunc& func);

    Viewport CreateViewport(const std::string name, uint32 width, uint32 height);
    std::vector<Viewport> GetViewports();
};

} // namespace CZ

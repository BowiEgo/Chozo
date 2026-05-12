#include <Runtime/RHI/CommandList.h>
#include <Runtime/RHI/CommandPool.h>
#include <Runtime/RenderCore/Renderer.h>

#include <Core/Log/LogMacros.h>

namespace CZ {

DEFINE_LOG_CATEGORY_STATIC(LogRenderer, Info);

struct FrameResource {
    CommandPool CommandPool;
    CommandList CommandList;
    // SyncObject RenderFence;
};

struct RendererObj {
    uint32 CurrentFrameIndex = 0;
    Window Window;
    GraphicsContext GraphicsContext;

    std::vector<FrameResource> Frames;
};

Renderer Renderer::Create(const RendererSpecification& spec) {
    auto ctx         = spec.GraphicsContext;
    RendererObj* obj = CZ_NEW(MEMORY_USAGE_RENDER, RendererObj);

    obj->Window          = spec.Window;
    obj->GraphicsContext = spec.GraphicsContext;

    obj->Frames.resize(ctx.GetMaxFramesInFlight());
    for (int i = 0; i < ctx.GetMaxFramesInFlight(); i++) {
        CommandPoolSpecification poolSpec;
        poolSpec.Flags             = CommandPoolFlags::ResetCommandBuffer;
        obj->Frames[i].CommandPool = ctx.GetDevice().CreateCommandPool(poolSpec);
        obj->Frames[i].CommandList = obj->Frames[i].CommandPool.AllocateCommandBuffer();
        // obj->Frames[i].RenderFence = IRHIAPI::CreateSyncObject();
    }

    return { obj };
}

void Renderer::Destroy(Renderer renderer) {
    RendererObj* obj = renderer;

    Delete(obj);
}

void Renderer::Init() { auto fbSize = m_Obj->Window.GetFrameBufferSize(); }

void Renderer::Tick(float deltaTime) {
    // UI::Get().Draw(cmdBuffer);
}

} // namespace CZ
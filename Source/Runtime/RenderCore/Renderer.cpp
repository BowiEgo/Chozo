#include <Runtime/RHI/CommandList.hpp>
#include <Runtime/RHI/CommandPool.hpp>
#include <Runtime/RHI/RHIAPI.hpp>
#include <Runtime/RenderCore/Renderer.hpp>

#include <Core/Log/LogMacros.hpp>
#include <cstddef>

namespace CZ {

DEFINE_LOG_CATEGORY_STATIC(LogRenderer, Info);

struct FrameResource {
    CommandPool CommandPool;
    CommandList CommandList;
};

struct RendererObj {
    uint32 CurrentFrameIndex = 0;
    Window Window;
    DrawFunc FinalPassDrawFunc;

    std::vector<FrameResource> Frames;
};

DEFINE_HANDLE_DESTROY(RendererObj)

Renderer Renderer::Create(const RendererSpecification& spec) {
    auto ctx         = RHIAPI::Get().GetGraphicsContext();
    RendererObj* obj = CZ_NEW(MEMORY_USAGE_RENDER, RendererObj);

    obj->Window = spec.Window;

    obj->Frames.resize(ctx.GetMaxFramesInFlight());
    for (uint32 i = 0; i < ctx.GetMaxFramesInFlight(); i++) {
        CommandPoolSpecification poolSpec;
        poolSpec.Flags             = CommandPoolFlags::ResetCommandBuffer;
        obj->Frames[i].CommandPool = ctx.GetDevice().CreateCommandPool(poolSpec);
        obj->Frames[i].CommandList = obj->Frames[i].CommandPool.AllocateCommandBuffer();
    }

    return { obj };
}

void Renderer::Shutdown() {
    RHIAPI::Get().WaitIdle();

    for (size_t i = 0; i < m_Obj->Frames.size(); i++) {
        m_Obj->Frames[i].CommandList.Destroy();
        m_Obj->Frames[i].CommandPool.Destroy();
    }

    m_Obj->Frames.clear();

    Destroy();
}

void Renderer::Tick(float deltaTime) {
    auto cmdList =
        m_Obj->Frames[RHIAPI::Get().GetGraphicsContext().GetCurrentFrameIndex()].CommandList;

    RHIAPI::Get().DrawFrame(cmdList, [&](uint32 imageIndex) {
        cmdList.Begin();

        {
            std::vector<Texture> targets;
            auto swapchainTexHandle =
                RHIAPI::Get().GetGraphicsContext().GetSwapchain().GetColorAttachment(imageIndex);

            targets.push_back(swapchainTexHandle);

            RHIAPI::Get().TransitionImageLayout(cmdList, swapchainTexHandle.GetImage(),
                                                ImageLayout::ColorAttachmentOptimal);

            RHIAPI::Get().BeginRendering(cmdList, targets,
                                         false); // bClear = false (to preserve the scene)

            if (m_Obj->FinalPassDrawFunc) m_Obj->FinalPassDrawFunc(cmdList);

            RHIAPI::Get().EndRendering(cmdList);

            RHIAPI::Get().TransitionImageLayout(cmdList, swapchainTexHandle.GetImage(),
                                                ImageLayout::PresentSrc);
        }

        cmdList.End();
    });

    RHIAPI::Get().GetGraphicsContext().End();
}

void Renderer::SetDrawFuncToFinalPass(const DrawFunc& func) { m_Obj->FinalPassDrawFunc = func; }

} // namespace CZ
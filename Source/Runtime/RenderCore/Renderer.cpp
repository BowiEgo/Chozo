#include <Runtime/RenderCore/Renderer.hpp>

#include <Runtime/RHI/CommandList.hpp>
#include <Runtime/RHI/CommandPool.hpp>
#include <Runtime/RHI/RHIAPI.hpp>
#include <Runtime/RenderCore/AssetManager.hpp>
#include <Runtime/RenderCore/Shader.hpp>
#include <Runtime/RenderCore/Viewport.hpp>

#include <Core/Log/LogMacros.hpp>
#include <Core/Memory/Memory.hpp>
#include <Core/Memory/MemoryTypes.hpp>

#include <cstddef>
#include <vector>

namespace CZ {

DEFINE_LOG_CATEGORY_STATIC(LogRenderer, Info);

static Shader testShader;
static Pipeline testPipeline;

template <> void Handle<RendererObj>::Destroy() {
    if (m_Obj) {
        for (auto& viewport : m_Obj->Viewports) {
            viewport.Destroy();
        }
        m_Obj->Viewports.clear();

        testShader.Destroy();
        testPipeline.Destroy();

        Delete(m_Obj);
        m_Obj = nullptr;
    }
}

Renderer Renderer::Create(const RendererSpecification& spec) {
    auto ctx         = RHIAPI::Get()->GetGraphicsContext();
    RendererObj* obj = CZ_NEW(MEMORY_USAGE_RENDER, RendererObj);

    obj->Window = spec.Window;

    obj->Frames.resize(ctx->GetMaxFramesInFlight());
    for (uint32 i = 0; i < ctx->GetMaxFramesInFlight(); i++) {
        CommandPoolSpecification poolSpec;
        poolSpec.Flags             = CommandPoolFlags::ResetCommandBuffer;
        obj->Frames[i].CommandPool = ctx->GetDevice()->CreateCommandPool(poolSpec);
        obj->Frames[i].CommandList = obj->Frames[i].CommandPool->AllocateCommandBuffer();
    }

    testShader = AssetManager::Get().GetOrLoadShader(
        { "Test", "shaders://Test.slang", { ShaderStage::Vertex, ShaderStage::Fragment }, "main" });

    auto testPipelineSpec         = PipelineSpecification{};
    testPipelineSpec.Name         = "TestPipeline";
    testPipelineSpec.ColorFormats = { PixelFormat::RGBA16F };

    testPipeline = ctx->GetDevice()->CreatePipeline(
        testPipelineSpec, testShader->GetShaderResources(), testShader->GetReflection());

    return { obj };
}

void Renderer::Shutdown() {
    RHIAPI::Get()->WaitIdle();

    for (size_t i = 0; i < m_Obj->Frames.size(); i++) {
        m_Obj->Frames[i].CommandList.Destroy();
        m_Obj->Frames[i].CommandPool.Destroy();
    }

    m_Obj->Frames.clear();

    Destroy();
}

void Renderer::Tick(float deltaTime) {
    auto cmdList =
        m_Obj->Frames[RHIAPI::Get()->GetGraphicsContext()->GetCurrentFrameIndex()].CommandList;

    RHIAPI::Get()->DrawFrame(cmdList, [&](uint32 imageIndex) {
        cmdList->Begin();

        for (auto& viewport : GetViewports()) {
            Texture viewportCanvas = viewport->GetFrameBuffer()->GetColorAttachment(0);
            auto width             = viewport->GetWidth();
            auto height            = viewport->GetHeight();

            std::vector<Texture> targets;

            targets.push_back(viewportCanvas);

            RHIAPI::Get()->TransitionImageLayout(cmdList, viewportCanvas->GetImage(),
                                                 ImageLayout::ColorAttachmentOptimal);

            cmdList->BindPipeline(testPipeline);

            RHIAPI::Get()->BeginRendering(cmdList, targets,
                                          false); // bClear = false (to preserve the scene)

            cmdList->SetViewport({ 0, 0, (float)width, (float)height, 0, 1 });
            cmdList->SetScissor({ 0, 0, width, height });

            cmdList->Draw(3, 1, 0, 0);

            RHIAPI::Get()->EndRendering(cmdList);

            RHIAPI::Get()->TransitionImageLayout(cmdList, viewportCanvas->GetImage(),
                                                 ImageLayout::ShaderReadOnlyOptimal);
        }

        {
            std::vector<Texture> targets;
            auto swapchainTexHandle =
                RHIAPI::Get()->GetGraphicsContext()->GetSwapchain()->GetColorAttachment(imageIndex);

            targets.push_back(swapchainTexHandle);

            RHIAPI::Get()->TransitionImageLayout(cmdList, swapchainTexHandle->GetImage(),
                                                 ImageLayout::ColorAttachmentOptimal);

            RHIAPI::Get()->BeginRendering(cmdList, targets,
                                          false); // bClear = false (to preserve the scene)

            if (m_Obj->FinalPassDrawFunc) m_Obj->FinalPassDrawFunc(cmdList);

            RHIAPI::Get()->EndRendering(cmdList);

            RHIAPI::Get()->TransitionImageLayout(cmdList, swapchainTexHandle->GetImage(),
                                                 ImageLayout::PresentSrc);
        }

        cmdList->End();
    });

    RHIAPI::Get()->GetGraphicsContext()->End();
}

void Renderer::SetDrawFuncToFinalPass(const DrawFunc& func) { m_Obj->FinalPassDrawFunc = func; }

Viewport Renderer::CreateViewport(const std::string name, uint32 width, uint32 height) {
    ViewportSpecification spec;
    spec.Name   = name;
    spec.Width  = width;
    spec.Height = height;

    auto viewport = Viewport::Create(spec);
    m_Obj->Viewports.push_back(viewport);

    return viewport;
}

std::vector<Viewport> Renderer::GetViewports() { return m_Obj->Viewports; }

} // namespace CZ
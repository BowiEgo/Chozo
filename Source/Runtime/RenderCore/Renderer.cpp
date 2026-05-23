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

DEFINE_HANDLE_DESTROY(RendererObj)

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

    auto testShader = AssetManager::Get().GetOrLoadShader(
        { "Test", "shaders://Test.slang", { ShaderStage::Vertex, ShaderStage::Fragment }, "main" });

    // // Pipeline
    // {

    //     PipelineSpecification spec;
    //     spec.Name               = "Test";
    //     spec.ShaderResources    = { testShader->GetShaderResources() };
    //     spec.ColorFormats       = { PixelFormat::RGBA16F };
    //     spec.VertexLayout       = { { ShaderDataType::Float3, "a_Position" },
    //                                 { ShaderDataType::Float3, "a_Normal" },
    //                                 { ShaderDataType::Float2, "a_TexCoord" },
    //                                 { ShaderDataType::Float3, "a_Tangent" },
    //                                 { ShaderDataType::Float3, "a_Bitangent" } };
    //     spec.PushConstantRanges = { { 0, sizeof(uint32_t) } };

    //     m_TestPipeline = RHIAPI::Get()->CreatePipeline(spec);
    // }

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

            std::vector<Texture> targets;

            targets.push_back(viewportCanvas);

            RHIAPI::Get()->TransitionImageLayout(cmdList, viewportCanvas->GetImage(),
                                                 ImageLayout::ColorAttachmentOptimal);

            RHIAPI::Get()->BeginRendering(cmdList, targets,
                                          false); // bClear = false (to preserve the scene)

            // cmdList->Draw(3, 1, 0, 0);

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
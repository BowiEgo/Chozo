#include <Runtime/App/Application.hpp>
#include <Runtime/RenderCore/Renderer.hpp>

#include <Runtime/App/Engine.hpp>
#include <Runtime/RHI/CommandList.hpp>
#include <Runtime/RHI/CommandPool.hpp>
#include <Runtime/RHI/RHIAPI.hpp>
#include <Runtime/RenderCore/Camera/CameraManager.hpp>
#include <Runtime/RenderCore/Shader.hpp>
#include <Runtime/RenderCore/Viewport.hpp>

#include <Runtime/RenderCore/MeshRegistry.hpp>
#include <Runtime/RenderCore/ProceduralMesh/CubeParamsObj.hpp>

#include <Core/Log/LogMacros.hpp>
#include <Core/Memory/Memory.hpp>
#include <Core/Memory/MemoryTypes.hpp>

#include <cstddef>
#include <vector>

namespace CZ {

Scope<RendererObj> Renderer::Create(const RendererSpecification& spec) {
    auto ctx = RHIAPI::Get()->GetGraphicsContext();
    auto obj = CZ_CREATE_SCOPE(MEMORY_USAGE_RENDER, RendererObj);

    obj->Window = spec.Window;

    obj->Frames.resize(ctx->GetMaxFramesInFlight());
    for (uint32 i = 0; i < ctx->GetMaxFramesInFlight(); i++) {
        CommandPoolSpecification poolSpec;
        poolSpec.Flags      = CommandPoolFlags::ResetCommandBuffer;
        obj->Frames[i].Pool = ctx->GetDevice()->CreateCommandPool(poolSpec);
        obj->Frames[i].List = obj->Frames[i].Pool->AllocateCommandBuffer();
    }

    auto testPipelineSpec         = PipelineSpecification{};
    testPipelineSpec.Name         = "TestPipeline";
    testPipelineSpec.ColorFormats = { PixelFormat::RGBA16F };

    const std::vector<std::string> files = { "shaders://Basic.slang" };

    // Compile the shaders on the job system and wait for the results before creating the pipeline.
    std::vector<std::future<Shader>> pendingShaders;
    for (auto& path : files) {
        pendingShaders.push_back(
            Application::Get().GetEngine()->GetShaderRegistry()->LoadAssetAsync(path));
    }

    for (auto& f : pendingShaders) {
        auto shader = f.get();
        CZ_CORE_LOG(Trace, "Shader {} compiled", shader.GetName());
        CZ_CORE_LOG(Trace, "Shader {} reflection: ", shader->GetReflection().ToString());

        if (shader.GetName() == "Basic")
            obj->TestPipeline = ctx->GetDevice()->CreatePipeline(
                testPipelineSpec, shader->GetShaderResources(), shader->GetReflection());
    }

    return obj;
}

void Renderer::Shutdown() {
    RHIAPI::Get()->WaitIdle();

    m_Obj->Frames.clear();
    m_Obj->Viewports.clear();
    m_Obj->TestPipeline.reset();

    CameraManager::Get().Shutdown();
}

void Renderer::Tick(float deltaTime) {
    RHIAPI::Get()->GetGraphicsContext()->SetCurrentFrame(deltaTime);

    auto cmdList = ViewAs<CommandList>(
        m_Obj->Frames[RHIAPI::Get()->GetGraphicsContext()->GetCurrentFrameIndex()].List);

    CameraManager::Get().UpdateAllCameras();

    RHIAPI::Get()->DrawFrame(cmdList, [&](uint32 imageIndex) {
        cmdList->Begin();

        for (auto& viewport : GetViewports()) {
            viewport->GetScene()->Update(deltaTime);

            Texture viewportCanvas = viewport->GetFrameBuffer()->GetColorAttachment(0);
            auto width             = viewport->GetWidth();
            auto height            = viewport->GetHeight();

            std::vector<Texture> targets;

            targets.push_back(viewportCanvas);

            RHIAPI::Get()->TransitionImageLayout(cmdList, viewportCanvas->GetImage(),
                                                 ImageLayout::ColorAttachmentOptimal);

            cmdList->BindPipeline(ViewAs<Pipeline>(m_Obj->TestPipeline));

            RHIAPI::Get()->BeginRendering(cmdList, targets,
                                          false); // bClear = false (to preserve the scene)

            cmdList->SetViewport({ 0, 0, (float)width, (float)height, 0, 1 });
            cmdList->SetScissor({ 0, 0, width, height });

            cmdList->Draw(viewport->GetScene(), viewport->GetCamera());

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

    m_Obj->Viewports.push_back(Viewport::Create(spec));

    return ViewAs<Viewport>(m_Obj->Viewports.back());
}

std::vector<Viewport> Renderer::GetViewports() {
    std::vector<Viewport> views;
    views.reserve(m_Obj->Viewports.size());

    for (const auto& viewport : m_Obj->Viewports) {
        views.push_back(ViewAs<Viewport>(viewport));
    }

    return views;
}

} // namespace CZ
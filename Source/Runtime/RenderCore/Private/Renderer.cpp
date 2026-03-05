#include "Renderer.h"

#include "ModuleUtils.h"
#include "RHIAPI.h"
#include "ShaderManager.h"

CRenderer::CRenderer(IRendererWindow* windowHandle) : m_Window(windowHandle) {}

CRenderer::~CRenderer() {}

void CRenderer::Init() {
    std::string libName = ChozoUitls::Module::GetPlatformLibName("Vulkan");
    if (m_RHIModule.Load(libName)) {
        FContextSpec spec;
        spec.FrameBufferSize = m_Window->GetFrameBufferSize();
        spec.NativeWindow = m_Window->GetNativeHandle();
        spec.WindowRequiredExtensions = m_Window->GetRequiredExtensions();

        m_GraphicContext = TScope<IRHIContext>(
            m_RHIModule.Invoke<IRHIContext*(const FContextSpec&)>("CreateVulkanContext", spec));
    }

    auto device = m_GraphicContext->GetDevice();

    CShaderManager::Init(device);

    FShaderSpecification vertShaderInfo("Test", "shaders://Test.glsl", EShaderStage::Vertex,
                                        "main");
    FShaderSpecification fagShaderInfo("Test", "shaders://Test.glsl", EShaderStage::Fragment,
                                       "main");
    TRef<CShader> vertShader = CShaderManager::Get()->Load(vertShaderInfo);
    TRef<CShader> fragShader = CShaderManager::Get()->Load(fagShaderInfo);

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        FCommandPoolSpecification poolSpec;

        poolSpec.Flags = ECommandPoolFlags::ResetCommandBuffer;
        m_Frames[i].CommandPool = device->CreateCommandPool(poolSpec);

        m_Frames[i].CommandBuffer = m_Frames[i].CommandPool->AllocateCommandBuffer();

        m_Frames[i].RenderFence = IRHIAPI::CreateSyncObject(m_GraphicContext.get());

        // m_SyncObjects[i] = IRHIAPI::CreateSyncObject(m_GraphicContext.get());
        // m_CommandBuffers[i] = IRHIAPI::CreateCommandBuffer(m_GraphicContext.get());
    }

    FFrameBufferSpecification fbSpec;
    fbSpec.Name = "SceneFrameBuffer";
    fbSpec.Size = m_Window->GetFrameBufferSize();
    fbSpec.ColorFormats = { EPixelFormat::RGBA8_UNORM };
    fbSpec.DepthFormat = EPixelFormat::D32_SFLOAT;

    m_SceneFrameBuffer = IRHIAPI::CreateFrameBuffer(m_GraphicContext.get(), fbSpec);

    FPipelineSpecification pipelineInfo;
    pipelineInfo.Name = "Test";
    pipelineInfo.RHIShaders = { vertShader->GetShaderResource(m_GraphicContext.get()),
                                fragShader->GetShaderResource(m_GraphicContext.get()) };
    pipelineInfo.ColorFormats = fbSpec.ColorFormats;
    m_ScenePipeline = IRHIAPI::CreatePipeline(m_GraphicContext.get(), pipelineInfo);
}

void CRenderer::Tick() {
    if (m_Window->CheckAndResetVSyncDirty()) {
        EPresentMode mode =
            m_Window->IsVSyncEnabled() ? EPresentMode::FIFO : EPresentMode::Immediate;

        m_GraphicContext->GetSwapchain()->SetPresentMode(mode);
    }

    auto& cmdBuffer = m_Frames[m_CurrentFrameIndex].CommandBuffer;
    auto& syncObject = m_Frames[m_CurrentFrameIndex].RenderFence;
    m_GraphicContext->SetCurrentFrameIndex(m_CurrentFrameIndex);
    m_GraphicContext->GetDevice()->TickDeferredDeletion(m_CurrentFrameIndex, MAX_FRAMES_IN_FLIGHT);

    IRHIAPI::DrawFrame(m_GraphicContext.get(), cmdBuffer, syncObject, [&](uint32 imageIndex) {
        cmdBuffer->Begin();

        // draw scene using RHI interface
        {
            auto target = m_SceneFrameBuffer->GetColorAttachment(0);
            auto extent = m_GraphicContext->GetSwapchain()->GetExtent();
            m_GraphicContext->SetTarget(target);

            IRHIAPI::BeginRendering(m_GraphicContext.get(), cmdBuffer, true);
            {
                cmdBuffer->BindPipeline(m_ScenePipeline);
                cmdBuffer->SetViewport(
                    { 0.0f, 0.0f, (float)extent.Width, (float)extent.Height, 0.0f, 1.0f });
                cmdBuffer->SetScissor({ 0, 0, extent.Width, extent.Height });
                cmdBuffer->Draw(3, 1, 0, 0);
            }
            IRHIAPI::EndRendering(m_GraphicContext.get(), cmdBuffer);

            IRHIAPI::PrepareTextureForSampling(m_GraphicContext.get(), cmdBuffer, target);
        }

        // draw UI on top of the scene
        {
            auto target = m_GraphicContext->GetSwapchain()->GetColorAttachment(imageIndex);
            m_GraphicContext->SetTarget(target);

            IRHIAPI::BeginRendering(m_GraphicContext.get(), cmdBuffer,
                                    false); // bClear = false (to preserve the scene)
            {
                if (m_UICallback) {
                    m_UICallback(cmdBuffer);
                }
            }
            IRHIAPI::EndRendering(m_GraphicContext.get(), cmdBuffer);
        }

        cmdBuffer->End();
    });

    m_CurrentFrameIndex = (m_CurrentFrameIndex + 1) % MAX_FRAMES_IN_FLIGHT;
}

void CRenderer::Shutdown() {
    if (!m_GraphicContext) return;

    m_GraphicContext->GetDevice()->WaitIdle();
    m_ScenePipeline = nullptr;
    m_SceneFrameBuffer = nullptr;

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        m_Frames[i].RenderFence = nullptr;
        m_Frames[i].CommandBuffer = nullptr;
        m_Frames[i].CommandPool = nullptr;
    }

    m_GraphicContext.reset();
}

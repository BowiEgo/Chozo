#include "Renderer.h"
#include "ShaderManager.h"

CRenderer::CRenderer(IRendererWindow* windowHandle) : m_Window(windowHandle) {}

CRenderer::~CRenderer() {}

void CRenderer::Init() {
    FRHIWindowInfo windowInfo;
    windowInfo.FrameBufferSize = m_Window->GetPhysicalSize();
    windowInfo.NativeWindow = m_Window->GetNativeHandle();
    windowInfo.RequiredExtensions = m_Window->GetRequiredExtensions();
    m_Context = CreateScope<CGraphicsContext>(windowInfo);

    auto RHI = m_Context->GetRHI();
    auto device = RHI->GetDevice();

    CShaderManager::Init(device);

    FShaderCreateInfo vertShaderInfo("Test", "shaders://Test.glsl", EShaderStage::Vertex, "main");
    FShaderCreateInfo fagShaderInfo("Test", "shaders://Test.glsl", EShaderStage::Fragment, "main");
    TRef<CShader> vertShader = CShaderManager::Get()->Load(vertShaderInfo);
    TRef<CShader> fragShader = CShaderManager::Get()->Load(fagShaderInfo);

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        m_SyncObjects[i] = RHI->CreateSyncObject();
        m_CommandBuffers[i] = RHI->CreateCommandBuffer();
    }

    FFrameBufferSpecification fbSpec;
    fbSpec.Name = "SceneFrameBuffer";
    fbSpec.Size = m_Window->GetPhysicalSize();
    fbSpec.ColorFormats = { EPixelFormat::RGBA8_UNORM };
    fbSpec.DepthFormat = EPixelFormat::D32_SFLOAT;

    m_SceneFrameBuffer = RHI->CreateFrameBuffer(fbSpec);

    FRHIPipelineCreateInfo pipelineInfo;
    pipelineInfo.Name = "Test";
    pipelineInfo.RHIShaders = { vertShader->GetRHIShader(), fragShader->GetRHIShader() };
    pipelineInfo.ColorFormats = fbSpec.ColorFormats;
    m_ScenePipeline = device->CreatePipeline(pipelineInfo);
}

void CRenderer::Tick() {
    auto& cmdBuffer = m_CommandBuffers[m_CurrentFrame];
    auto& syncObject = m_SyncObjects[m_CurrentFrame];
    auto RHI = m_Context->GetRHI();

    RHI->DrawFrame(cmdBuffer, syncObject, m_CurrentFrame, [&](uint32 imageIndex) {
        cmdBuffer->Begin();

        // draw scene using RHI interface
        {
            auto target = m_SceneFrameBuffer->GetColorAttachment(0);
            auto extent = RHI->GetSwapchain()->GetExtent();
            RHI->BeginRendering(cmdBuffer, target, true);
            {
                cmdBuffer->BindPipeline(m_ScenePipeline);
                cmdBuffer->SetViewport(
                    { 0.0f, 0.0f, (float)extent.Width, (float)extent.Height, 0.0f, 1.0f });
                cmdBuffer->SetScissor({ 0, 0, extent.Width, extent.Height });
                cmdBuffer->Draw(3, 1, 0, 0);
            }
            RHI->EndRendering(cmdBuffer);

            RHI->PrepareTextureForSampling(cmdBuffer, target);
        }

        // draw UI on top of the scene
        {
            auto target = RHI->GetSwapchain()->GetColorAttachment(imageIndex);
            RHI->BeginRendering(cmdBuffer, target, false); // bClear = false (to preserve the scene)
            {
                if (m_UICallback) {
                    m_UICallback(cmdBuffer);
                }
            }
            RHI->EndRendering(cmdBuffer);
        }

        cmdBuffer->End();
    });

    m_CurrentFrame = (m_CurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void CRenderer::Shutdown() {
    if (!m_Context) return;

    m_Context->GetRHI()->GetDevice()->WaitIdle();
    m_ScenePipeline = nullptr;
    m_SceneFrameBuffer = nullptr;

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        m_SyncObjects[i] = nullptr;
        m_CommandBuffers[i] = nullptr;
    }

    m_Context.reset();
}

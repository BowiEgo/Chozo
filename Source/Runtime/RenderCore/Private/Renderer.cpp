#include "Renderer.h"
#include "ShaderManager.h"

CRenderer::CRenderer(IRendererWindow* windowHandle) : m_Window(windowHandle) {}

CRenderer::~CRenderer() {}

void CRenderer::Init() {
    FRHIWindowInfo windowInfo;
    windowInfo.FrameBufferSize = m_Window->GetFramebufferSize();
    windowInfo.NativeWindow = m_Window->GetNativeHandle();
    windowInfo.RequiredExtensions = m_Window->GetRequiredExtensions();
    m_Context = CreateScope<CGraphicsContext>(windowInfo);

    CShaderManager::Init(m_Context->GetRHI()->GetDevice());

    FShaderCreateInfo vertShaderInfo("Test", "shaders://Test.glsl", EShaderStage::Vertex, "main");
    FShaderCreateInfo fagShaderInfo("Test", "shaders://Test.glsl", EShaderStage::Fragment, "main");
    TRef<CShader> vertShader = CShaderManager::Get()->Load(vertShaderInfo);
    TRef<CShader> fragShader = CShaderManager::Get()->Load(fagShaderInfo);

    FRHIPipelineCreateInfo pipelineInfo;
    pipelineInfo.Name = "Test";
    pipelineInfo.RHIShaders = {vertShader->GetRHIShader(), fragShader->GetRHIShader()};
    pipelineInfo.ColorFormats.push_back(m_Context->GetRHI()->GetSwapchain()->GetImageFormat());
    m_ScenePipeline = m_Context->GetRHI()->GetDevice()->CreatePipeline(pipelineInfo);

    // m_Context->SetPipeline(m_ScenePipeline);

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        m_SyncObjects[i] = m_Context->GetRHI()->CreateSyncObject();
        m_CommandBuffers[i] = m_Context->GetRHI()->CreateCommandBuffer();
    }
}

void CRenderer::Tick() {
    auto& cmdBuffer = m_CommandBuffers[m_CurrentFrame];
    auto& syncObject = m_SyncObjects[m_CurrentFrame];
    auto RHI = m_Context->GetRHI();

    // m_Context->DrawFrame(cmdBuffer, syncObject);
    RHI->DrawFrame(cmdBuffer, syncObject, [&](uint32 imageIndex) {
        cmdBuffer->Begin();

        RHI->BeginRenderingToSwapchain(cmdBuffer, imageIndex,
                                       true); // bClear = true / false(preserve the scene)
        {
            // 1. draw scene using RHI interface
            auto extent = RHI->GetSwapchain()->GetExtent();
            cmdBuffer->BindPipeline(m_ScenePipeline);
            cmdBuffer->SetViewport(
                {0.0f, 0.0f, (float)extent.Width, (float)extent.Height, 0.0f, 1.0f});
            cmdBuffer->SetScissor({0, 0, extent.Width, extent.Height});
            cmdBuffer->Draw(3, 1, 0, 0);

            // 2. draw UI on top of the scene
            if (m_UICallback) {
                m_UICallback(cmdBuffer);
            }
        }
        RHI->EndRendering(cmdBuffer);

        cmdBuffer->End();
    });

    m_CurrentFrame = (m_CurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void CRenderer::Shutdown() {
    if (!m_Context)
        return;

    m_Context->GetRHI()->GetDevice()->WaitIdle();
    m_ScenePipeline = nullptr;

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        m_SyncObjects[i] = nullptr;
        m_CommandBuffers[i] = nullptr;
    }

    m_Context.reset();
}

#include "Renderer.h"
#include "ModuleUtils.h"
#include "ShaderManager.h"

CRenderer::CRenderer(IRendererWindow* windowHandle) : m_Window(windowHandle) {}

CRenderer::~CRenderer() {}

void CRenderer::Init() {
    if (m_RHIModule.Load("VulkanRHI.dll")) {
        FRHIWindowInfo windowInfo;
        windowInfo.FrameBufferSize = m_Window->GetFramebufferSize();
        windowInfo.NativeWindow = m_Window->GetNativeHandle();
        windowInfo.RequiredExtensions = m_Window->GetRequiredExtensions();

        m_RHIModule.Load(GetPlatformLibName("VulkanRHI"));
        m_Context =
            TScope<IGraphicsContext>(m_RHIModule.Invoke<IGraphicsContext*(const FRHIWindowInfo&)>(
                "CreateVulkanGraphicsContext", windowInfo));

        CShaderManager::Init(m_Context->GetRHI()->GetDevice());

        FShaderCreateInfo vertShaderInfo("Test", "shaders://Test.glsl", EShaderStage::Vertex,
                                         "main");
        FShaderCreateInfo fagShaderInfo("Test", "shaders://Test.glsl", EShaderStage::Fragment,
                                        "main");
        TRef<CShader> vertShader = CShaderManager::Get()->Load(vertShaderInfo);
        TRef<CShader> fragShader = CShaderManager::Get()->Load(fagShaderInfo);

        FRHIPipelineCreateInfo pipelineInfo;
        pipelineInfo.Name = "Test";
        pipelineInfo.RHIShaders = {vertShader->GetRHIShader(), fragShader->GetRHIShader()};
        pipelineInfo.ColorFormats.push_back(m_Context->GetRHI()->GetSwapchain()->GetImageFormat());
        auto pipeline = m_Context->GetRHI()->GetDevice()->CreatePipeline(pipelineInfo);

        m_Context->SetPipeline(pipeline);

        for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            m_SyncObjects[i] = m_Context->GetRHI()->CreateSyncObject();
            m_CommandBuffers[i] = m_Context->GetRHI()->CreateCommandBuffer();
        }
    }
}

void CRenderer::Tick() {
    auto& cmdBuffer = m_CommandBuffers[m_CurrentFrame];
    auto& syncObject = m_SyncObjects[m_CurrentFrame];

    m_Context->DrawFrame(cmdBuffer, syncObject);

    m_CurrentFrame = (m_CurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void CRenderer::Shutdown() {
    if (m_Context) {
        m_Context->GetRHI()->GetDevice()->WaitIdle();

        for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            m_SyncObjects[i] = nullptr;
            m_CommandBuffers[i] = nullptr;
        }

        m_Context.reset();
    }
}

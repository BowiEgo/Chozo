#include "Renderer.h"

#include "CameraUniformManager.h"
#include "ModuleUtils.h"
#include "RHIAPI.h"
#include "ShaderManager.h"

CRenderer::CRenderer(IRendererWindow* windowHandle) : m_Window(windowHandle) {}

CRenderer::~CRenderer() {}

void CRenderer::Init() {
    auto fbSize = m_Window->GetFrameBufferSize();

    std::string libName = ChozoUitls::Module::GetPlatformLibName("VulkanRHI");
    if (m_RHIModule.Load(libName)) {
        FContextSpec spec;
        spec.FrameBufferSize = fbSize;
        spec.NativeWindow = m_Window->GetNativeHandle();
        spec.WindowRequiredExtensions = m_Window->GetRequiredExtensions();

        m_GraphicContext = TScope<IRHIContext>(
            m_RHIModule.Invoke<IRHIContext*(const FContextSpec&)>("CreateVulkanContext", spec));
    }

    m_Frames.reserve(m_GraphicContext->GetMaxFramesInFlight());

    auto device = m_GraphicContext->GetDevice();

    // Camera
    CCameraUniformManager::Get().Initialize(m_GraphicContext.get());

    // Shader
    CShaderManager::Init(device);

    FShaderSpecification vertShaderInfo("Test", "shaders://Test.glsl", EShaderStage::Vertex,
                                        "main");
    FShaderSpecification fagShaderInfo("Test", "shaders://Test.glsl", EShaderStage::Fragment,
                                       "main");
    TRef<CShader> vertShader = CShaderManager::Get()->Load(vertShaderInfo);
    TRef<CShader> fragShader = CShaderManager::Get()->Load(fagShaderInfo);

    for (int i = 0; i < m_GraphicContext->GetMaxFramesInFlight(); i++) {
        FCommandPoolSpecification poolSpec;

        poolSpec.Flags = ECommandPoolFlags::ResetCommandBuffer;
        m_Frames[i].CommandPool = device->CreateCommandPool(poolSpec);

        m_Frames[i].CommandBuffer = m_Frames[i].CommandPool->AllocateCommandBuffer();

        m_Frames[i].RenderFence = IRHIAPI::CreateSyncObject(m_GraphicContext.get());
    }

    FPipelineSpecification pipelineInfo;
    pipelineInfo.Name = "Test";
    pipelineInfo.RHIShaders = { vertShader->GetShaderResource(m_GraphicContext.get()),
                                fragShader->GetShaderResource(m_GraphicContext.get()) };
    pipelineInfo.ColorFormats = { EPixelFormat::RGBA8_UNORM };
    m_ScenePipeline = IRHIAPI::CreatePipeline(m_GraphicContext.get(), pipelineInfo);
}

void CRenderer::Tick(float deltaTime) {
    if (m_Window->CheckAndResetVSyncDirty()) {
        EPresentMode mode =
            m_Window->IsVSyncEnabled() ? EPresentMode::FIFO : EPresentMode::Immediate;

        m_GraphicContext->GetSwapchain()->SetPresentMode(mode);
    }

    auto& cmdBuffer = m_Frames[m_CurrentFrameIndex].CommandBuffer;
    auto& syncObject = m_Frames[m_CurrentFrameIndex].RenderFence;
    m_GraphicContext->SetCurrentFrameIndex(m_CurrentFrameIndex);
    m_GraphicContext->GetDevice()->TickDeferredDeletion(m_CurrentFrameIndex);
    CCameraUniformManager::Get().UpdateAllCameras();

    IRHIAPI::DrawFrame(m_GraphicContext.get(), cmdBuffer, syncObject, [&](uint32 imageIndex) {
        cmdBuffer->Begin();

        for (auto& viewport : m_Viewports) {
            viewport->BeginRender(cmdBuffer.get(), m_ScenePipeline);
            cmdBuffer->Draw(3, 1, 0, 0);
            viewport->EndRender(cmdBuffer.get());
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

    m_CurrentFrameIndex = (m_CurrentFrameIndex + 1) % m_GraphicContext->GetMaxFramesInFlight();
}

void CRenderer::Shutdown() {
    if (!m_GraphicContext) return;

    m_GraphicContext->GetDevice()->WaitIdle();

    m_Viewports.clear();
    m_ScenePipeline = nullptr;

    for (int i = 0; i < m_GraphicContext->GetMaxFramesInFlight(); i++) {
        m_Frames[i].RenderFence = nullptr;
        m_Frames[i].CommandBuffer = nullptr;
        m_Frames[i].CommandPool = nullptr;
    }

    CCameraUniformManager::Get().Shutdown();

    m_GraphicContext.reset();
}

CViewport* CRenderer::CreateViewport(const std::string name, uint32 width, uint32 height) {
    FViewportSpecification spec;
    spec.Name = name;
    spec.Width = width;
    spec.Height = height;

    auto viewport = CreateScope<CViewport>(m_GraphicContext.get(), spec);
    auto* ptr = viewport.get();
    m_Viewports.push_back(std::move(viewport));

    return ptr;
}
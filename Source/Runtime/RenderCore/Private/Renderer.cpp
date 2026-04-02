#include "Renderer.h"

#include "CameraUniformManager.h"
#include "MeshManager.h"
#include "ModuleUtils.h"
#include "RHIAPI.h"
#include "ShaderManager.h"

CRenderer::CRenderer(IRendererWindow* windowHandle) : m_Window(windowHandle) {}

CRenderer::~CRenderer() {}

void CRenderer::Init() {
    FMeshRegister::Init();
    FLightRegister::Init();

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

    auto device = m_GraphicContext->GetDevice();

    m_Frames.resize(m_GraphicContext->GetMaxFramesInFlight());
    for (int i = 0; i < m_GraphicContext->GetMaxFramesInFlight(); i++) {
        FCommandPoolSpecification poolSpec;
        poolSpec.Flags = ECommandPoolFlags::ResetCommandBuffer;
        m_Frames[i].CommandPool = device->CreateCommandPool(poolSpec);
        m_Frames[i].CommandList = m_Frames[i].CommandPool->AllocateCommandBuffer();
        m_Frames[i].RenderFence = IRHIAPI::CreateSyncObject(m_GraphicContext.get());
    }

    // Camera
    CCameraUniformManager::Get().Initialize(m_GraphicContext.get());

    // Shader
    CShaderManager::Init(device);

    FShaderSpecification vertShaderInfo("Cube", "shaders://Cube.glsl", EShaderStage::Vertex,
                                        "main");
    FShaderSpecification fagShaderInfo("Cube", "shaders://Cube.glsl", EShaderStage::Fragment,
                                       "main");
    TRef<CShader> vertShader = CShaderManager::Get()->Load(vertShaderInfo);
    TRef<CShader> fragShader = CShaderManager::Get()->Load(fagShaderInfo);

    // Pipeline
    {
        FPipelineSpecification solidSpec;
        solidSpec.Name = "Solid";
        solidSpec.RHIShaders = { vertShader->GetShaderResource(m_GraphicContext.get()),
                                 fragShader->GetShaderResource(m_GraphicContext.get()) };
        solidSpec.ColorFormats = { EPixelFormat::RGBA8_UNORM };
        solidSpec.VertexLayout = { { EShaderDataType::Float3, "a_Position" },
                                   { EShaderDataType::Float3, "a_Normal" },
                                   { EShaderDataType::Float2, "a_TexCoord" },
                                   { EShaderDataType::Float3, "a_Tangent" },
                                   { EShaderDataType::Float3, "a_Bitangent" } };
        solidSpec.PushConstantRanges = { { 0, sizeof(FMatrix4) + sizeof(FMatrix3) } };
        m_SolidPipeline = IRHIAPI::CreatePipeline(m_GraphicContext.get(), solidSpec);

        FPipelineSpecification wireSpec = solidSpec;
        wireSpec.Name = "Wireframe";
        wireSpec.PolygonMode = EPolygonMode::Line;
        m_WireframePipeline = IRHIAPI::CreatePipeline(m_GraphicContext.get(), wireSpec);
    }

    SetPolygonMode(EPolygonMode::Fill);

    // m_Cube = CreateRef<FCube>();
    // m_Cube->Upload(m_GraphicContext.get());

    // m_Sphere = CreateRef<FSphere>();
    // m_Sphere->Upload(m_GraphicContext.get());
}

void CRenderer::Tick(float deltaTime) {
    if (m_Window->CheckAndResetVSyncDirty()) {
        EPresentMode mode =
            m_Window->IsVSyncEnabled() ? EPresentMode::FIFO : EPresentMode::Immediate;

        m_GraphicContext->GetSwapchain()->SetPresentMode(mode);
    }

    auto& cmdList = m_Frames[m_CurrentFrameIndex].CommandList;
    auto& syncObject = m_Frames[m_CurrentFrameIndex].RenderFence;
    m_GraphicContext->SetCurrentFrameIndex(m_CurrentFrameIndex);
    m_GraphicContext->GetDevice()->TickDeferredDeletion(m_CurrentFrameIndex);
    CCameraUniformManager::Get().UpdateAllCameras();

    IRHIAPI::DrawFrame(m_GraphicContext.get(), cmdList, syncObject, [&](uint32 imageIndex) {
        cmdList->Begin();

        for (auto& viewport : m_Viewports) {
            viewport->BeginRender(cmdList.get(), m_CurrentPipeline);
            // cmdList->Draw(3, 1, 0, 0);
            // m_Cube->Draw(cmdList.get());
            // m_Sphere->Draw(cmdList.get());
            // cmdList->BindVertexBuffer(m_Cube->GetVertexBuffer(), 0); // binding = 0
            // cmdList->BindIndexBuffer(m_Cube->GetIndexBuffer());
            // cmdList->DrawIndexed(m_Cube->GetIndexCount(), 1, 0, 0, 0);
            viewport->EndRender(cmdList.get());
        }

        // draw UI on top of the scene
        {
            auto target = m_GraphicContext->GetSwapchain()->GetColorAttachment(imageIndex);
            m_GraphicContext->SetTarget(target);

            IRHIAPI::BeginRendering(m_GraphicContext.get(), cmdList,
                                    false); // bClear = false (to preserve the scene)
            {
                if (m_UICallback) {
                    m_UICallback(cmdList);
                }
            }
            IRHIAPI::EndRendering(m_GraphicContext.get(), cmdList);
        }

        cmdList->End();
    });

    m_CurrentFrameIndex = (m_CurrentFrameIndex + 1) % m_GraphicContext->GetMaxFramesInFlight();
}

void CRenderer::Shutdown() {
    if (!m_GraphicContext) return;

    m_GraphicContext->GetDevice()->WaitIdle();

    // m_Cube = nullptr;
    // m_Sphere = nullptr;

    m_Viewports.clear();
    m_SolidPipeline = nullptr;
    m_WireframePipeline = nullptr;
    m_CurrentPipeline = nullptr;

    for (int i = 0; i < m_GraphicContext->GetMaxFramesInFlight(); i++) {
        m_Frames[i].RenderFence = nullptr;
        m_Frames[i].CommandList = nullptr;
        m_Frames[i].CommandPool = nullptr;
    }

    CCameraUniformManager::Get().Shutdown();
    FMeshManager::Get().Shutdown();

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
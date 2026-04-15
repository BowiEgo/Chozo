#include "Renderer.h"

#include "AssetManager.h"
#include "CameraUniformManager.h"
#include "MeshManager.h"
#include "ModuleUtils.h"
#include "RHIAPI.h"
#include "RenderGraph.h"

CRenderer::CRenderer(IRendererWindow* windowHandle) : m_Window(windowHandle) {}

CRenderer::~CRenderer() {}

void CRenderer::Init() {
    FMeshRegister::Init();
    FLightRegister::Init();

    auto fbSize = m_Window->GetFrameBufferSize();

    std::string libName = ChozoUitls::Module::GetPlatformLibName("VulkanRHI");
    if (m_RHIModule.Load(libName)) {
        FContextSpec spec;
        spec.FrameBufferSize          = fbSize;
        spec.NativeWindow             = m_Window->GetNativeHandle();
        spec.WindowRequiredExtensions = m_Window->GetRequiredExtensions();

        m_GraphicContext = TScope<IRHIContext>(
            m_RHIModule.Invoke<IRHIContext*(const FContextSpec&)>("CreateVulkanContext", spec));

        IRHIAPI::SetContext(m_GraphicContext.get());
    }

    auto device = m_GraphicContext->GetDevice();

    m_Frames.resize(m_GraphicContext->GetMaxFramesInFlight());
    for (int i = 0; i < m_GraphicContext->GetMaxFramesInFlight(); i++) {
        FCommandPoolSpecification poolSpec;
        poolSpec.Flags          = ECommandPoolFlags::ResetCommandBuffer;
        m_Frames[i].CommandPool = device->CreateCommandPool(poolSpec);
        m_Frames[i].CommandList = m_Frames[i].CommandPool->AllocateCommandBuffer();
        m_Frames[i].RenderFence = IRHIAPI::CreateSyncObject();
    }

    // Camera
    CCameraUniformManager::Get().Initialize();

    // Shader
    TRef<CShader> cubeShader =
        CAssetManager::Get().GetOrLoadShader({ "Cube",
                                               "shaders://Cube.glsl",
                                               { EShaderStage::Vertex, EShaderStage::Fragment },
                                               "main" });

    TRef<CShader> cubemapSamplerShader =
        CAssetManager::Get().GetOrLoadShader({ "CubemapSampler",
                                               "shaders://CubemapSampler.glsl",
                                               { EShaderStage::Vertex, EShaderStage::Fragment },
                                               "main" });

    // Texture
    m_SkyboxTex = CAssetManager::Get().GetOrLoadTexture(
        "/Volumes/WD My Passport/ChozoProjectReSources/Textures/HDRI/newport_loft.hdr");

    // Pipeline
    {
        FPipelineSpecification solidSpec;
        solidSpec.Name               = "Solid";
        solidSpec.RHIShaders         = cubeShader->GetShaderResources();
        solidSpec.ColorFormats       = { EPixelFormat::RGBA8_UNORM };
        solidSpec.VertexLayout       = { { EShaderDataFormat::Float3, "a_Position" },
                                         { EShaderDataFormat::Float3, "a_Normal" },
                                         { EShaderDataFormat::Float2, "a_TexCoord" },
                                         { EShaderDataFormat::Float3, "a_Tangent" },
                                         { EShaderDataFormat::Float3, "a_Bitangent" } };
        solidSpec.PushConstantRanges = { { 0, sizeof(FMatrix4) + sizeof(FMatrix3) } };
        m_SolidPipeline              = IRHIAPI::CreatePipeline(solidSpec);

        FPipelineSpecification wireSpec = solidSpec;
        wireSpec.Name                   = "Wireframe";
        wireSpec.PolygonMode            = EPolygonMode::Line;
        m_WireframePipeline             = IRHIAPI::CreatePipeline(wireSpec);
    }

    {

        FPipelineSpecification spec;
        spec.Name         = "CubemapSampler";
        spec.RHIShaders   = { cubemapSamplerShader->GetShaderResources() };
        spec.ColorFormats = { EPixelFormat::RGBA16F };
        spec.VertexLayout = {
            { EShaderDataFormat::Float3, "a_Position" },
        };
        m_CubemapSamplerPipeline = IRHIAPI::CreatePipeline(spec);
    }

    SetPolygonMode(EPolygonMode::Fill);

    m_Cube = CreateRef<FCube>();
    m_Cube->Upload();
}

void CRenderer::Tick(float deltaTime) {
    if (m_Window->CheckAndResetVSyncDirty()) {
        EPresentMode mode =
            m_Window->IsVSyncEnabled() ? EPresentMode::FIFO : EPresentMode::Immediate;

        m_GraphicContext->GetSwapchain()->SetPresentMode(mode);
    }

    auto& cmdList    = m_Frames[m_CurrentFrameIndex].CommandList;
    auto& syncObject = m_Frames[m_CurrentFrameIndex].RenderFence;
    m_GraphicContext->SetCurrentFrameIndex(m_CurrentFrameIndex);
    m_GraphicContext->GetDevice()->TickDeferredDeletion(m_CurrentFrameIndex);
    CCameraUniformManager::Get().UpdateAllCameras();

    IRHIAPI::DrawFrame(cmdList, syncObject, [&](uint32 imageIndex) {
        cmdList->Begin();

        // ------ Render Graph ------
        CRenderGraph graph(m_GraphicContext.get());

        FRDGTexture* skybox2DHandle = graph.ImportExternalRDGTexture(
            "SkyboxTexture", m_SkyboxTex->GetOrCreateResource(),
            EImageLayout::ShaderReadOnlyOptimal, EImageLayout::ShaderReadOnlyOptimal);

        FTextureSpecification cubeSpec;
        cubeSpec.Type   = ETextureType::TextureCube;
        cubeSpec.Size   = { 512, 512 };
        cubeSpec.Format = EPixelFormat::RGBA16F;
        cubeSpec.Usage  = ETextureUsage::Texture | ETextureUsage::Attachment;

        uint32_t faceSize = cubeSpec.Size.Width;

        TScope<IRHITexture> skyboxCubemap = IRHIAPI::CreateTexture(cubeSpec, nullptr);
        FRDGTexture* skyboxCubemapHandle =
            graph.CreateRDGTexture("SkyboxCubemap", skyboxCubemap.get());

        graph.AddPass("EquirectangularToCubemap", { skybox2DHandle }, { skyboxCubemapHandle },
                      ERenderPassLoadOp::Clear, [this, faceSize, skybox2DHandle](CRDGContext& ctx) {
                          auto st = ctx.GetTexture(skybox2DHandle);

                          auto cmd = ctx.GetCommandBuffer();
                          cmd->BindPipeline(m_CubemapSamplerPipeline);
                          cmd->BindTexture(st, 0, 0);
                          cmd->SetViewport({ 0, 0, (float)faceSize, (float)faceSize, 0, 1 });
                          cmd->SetScissor({ 0, 0, faceSize, faceSize });

                          m_Cube->Draw(cmd.get());
                      });

        // Request a transient texture for storing the skybox pass result,
        // lifetime managed by the render graph
        for (auto& viewport : m_Viewports) {
            IRHITexture* viewportCanvas = viewport->GetFrameBuffer()->GetColorAttachment(0).get();
            // auto tex = CreateRef<CTexture>(viewportCanvas->GetSpec(), viewportCanvas);

            FRDGTexture* viewportHandle = graph.ImportExternalRDGTexture(
                "ViewportCanvas_" + viewport->GetName(), viewportCanvas,
                EImageLayout::ColorAttachmentOptimal, // initial usage
                EImageLayout::ShaderReadOnlyOptimal); // final usage (for UI)

            // graph.AddPass("SkyboxPass", { skyboxHandle }, { viewportHandle },
            //               ERenderPassLoadOp::Clear, [=](CRDGContext& ctx) {
            //                   TRef<IRHITexture> st = ctx.GetTextureCube(skyboxHandle);
            //                   TRef<IRHITexture> rt      = ctx.GetTexture2D(viewportHandle);

            //                   DrawSkybox(ctx.GetCommandBuffer(), st, rt);
            //               });

            graph.AddPass("SceneCompositePass", {}, { viewportHandle }, ERenderPassLoadOp::Clear,
                          [this, &viewport, viewportHandle](CRDGContext& ctx) {
                              //   auto rt  = ctx.GetTexture(viewportHandle);
                              auto cmd = ctx.GetCommandBuffer();

                              auto scene  = viewport->GetScene();
                              auto camera = viewport->GetCamera();
                              auto uniformBuffer =
                                  CCameraUniformManager::Get().GetBufferForCamera(camera.get());
                              auto width  = viewport->GetWidth();
                              auto height = viewport->GetHeight();

                              cmd->BindPipeline(m_CurrentPipeline);
                              cmd->BindUniformBuffer(uniformBuffer, 0, 0);
                              cmd->SetViewport({ 0, 0, (float)width, (float)height, 0, 1 });
                              cmd->SetScissor({ 0, 0, width, height });

                              scene->Draw(cmd.get());
                          });
        }

        // // draw UI on top of the scene
        IRHITexture* backbuffer =
            m_GraphicContext->GetSwapchain()->GetColorAttachment(imageIndex).get();
        // auto tex = CreateRef<CTexture>(backbuffer->GetSpec(), backbuffer);

        FRDGTexture* BackbufferHandle = graph.ImportExternalRDGTexture(
            "Backbuffer", backbuffer, EImageLayout::ColorAttachmentOptimal, // initial usage
            EImageLayout::PresentSrc);

        graph.AddPass("SwapchainPass", {}, { BackbufferHandle }, ERenderPassLoadOp::Clear,
                      [this](CRDGContext& ctx) {
                          if (m_UICallback) {
                              m_UICallback(ctx.GetCommandBuffer());
                          }
                      });

        graph.Compile(); // Request physical resources from the global singleton ImagePool
        graph.Execute(cmdList.get());
        // ------ Render Graph End ------

        cmdList->End();
    });

    m_CurrentFrameIndex = (m_CurrentFrameIndex + 1) % m_GraphicContext->GetMaxFramesInFlight();
}

void CRenderer::Shutdown() {
    if (!m_GraphicContext) return;

    m_GraphicContext->GetDevice()->WaitIdle();

    m_SkyboxTex.Reset(); // TODO: Remove

    m_Viewports.clear();
    m_SolidPipeline.Reset();
    m_WireframePipeline.Reset();
    m_CurrentPipeline.Reset();
    m_CubemapSamplerPipeline.Reset();
    m_SkyboxPipeline.Reset();

    for (int i = 0; i < m_GraphicContext->GetMaxFramesInFlight(); i++) {
        m_Frames[i].RenderFence.Reset();
        m_Frames[i].CommandList.Reset();
        m_Frames[i].CommandPool.Reset();
    }

    CCameraUniformManager::Get().Shutdown();
    FMeshManager::Get().Shutdown();
    CAssetManager::Get().Shutdown();

    m_GraphicContext.reset();
}

CViewport* CRenderer::CreateViewport(const std::string name, uint32 width, uint32 height) {
    FViewportSpecification spec;
    spec.Name   = name;
    spec.Width  = width;
    spec.Height = height;

    auto viewport = CreateScope<CViewport>(spec);
    auto* ptr     = viewport.get();
    m_Viewports.push_back(std::move(viewport));

    return ptr;
}
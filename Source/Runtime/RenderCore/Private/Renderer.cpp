#include "Renderer.h"

#include "AssetManager.h"
#include "CameraUniformManager.h"
#include "MeshManager.h"
#include "ModuleUtils.h"
#include "RHIAPI.h"
#include "RHIDescriptorSet.h"
#include "RenderGraph.h"

#include "MathUtils.h" // TODO: Remove

static const FMatrix4 s_CubeTextureCaptureViews[] = {
    FMatrix4::LookAt(FVector3(0.0f), FVector3::Right, FVector3::Down),    // +X
    FMatrix4::LookAt(FVector3(0.0f), FVector3::Left, FVector3::Down),     // -X
    FMatrix4::LookAt(FVector3(0.0f), FVector3::Up, FVector3::Backward),   // +Y
    FMatrix4::LookAt(FVector3(0.0f), FVector3::Down, FVector3::Forward),  // -Y
    FMatrix4::LookAt(FVector3(0.0f), FVector3::Backward, FVector3::Down), // +Z
    FMatrix4::LookAt(FVector3(0.0f), FVector3::Forward, FVector3::Down)   // -Z
};
static const FMatrix4 s_CubeTextureCaptureProjection =
    FMatrix4::Perspective(90.0f, 1.0f, 0.01f, 1000.0f);

CRenderer::CRenderer(IRendererWindow* windowHandle)
    : m_Window(windowHandle), m_Profiler(CreateScope<PerformanceProfiler>(
                                  static_cast<uint32_t>(ERendererProfileSlot::COUNT))) {}

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

    TRef<CShader> gBufferShader =
        CAssetManager::Get().GetOrLoadShader({ "GBuffer",
                                               "shaders://GBuffer.glsl",
                                               { EShaderStage::Vertex, EShaderStage::Fragment },
                                               "main" });
    TRef<CShader> debugShader =
        CAssetManager::Get().GetOrLoadShader({ "Debug",
                                               "shaders://Debug.glsl",
                                               { EShaderStage::Vertex, EShaderStage::Fragment },
                                               "main" });

    TRef<CShader> pbrShader = CAssetManager::Get().GetOrLoadShader(
        { "PBR", "shaders://PBR.glsl", { EShaderStage::Vertex, EShaderStage::Fragment }, "main" });

    TRef<CShader> cubemapSamplerShader =
        CAssetManager::Get().GetOrLoadShader({ "CubemapSampler",
                                               "shaders://CubemapSampler.glsl",
                                               { EShaderStage::Vertex, EShaderStage::Fragment },
                                               "main" });

    TRef<CShader> irradianceShader =
        CAssetManager::Get().GetOrLoadShader({ "IrradianceConvolution",
                                               "shaders://IrradianceConvolution.glsl",
                                               { EShaderStage::Vertex, EShaderStage::Fragment },
                                               "main" });

    TRef<CShader> prefilteredShader =
        CAssetManager::Get().GetOrLoadShader({ "PrefilteredSampler",
                                               "shaders://PrefilteredSampler.glsl",
                                               { EShaderStage::Vertex, EShaderStage::Fragment },
                                               "main" });

    TRef<CShader> skyboxShader =
        CAssetManager::Get().GetOrLoadShader({ "Skybox",
                                               "shaders://Skybox.glsl",
                                               { EShaderStage::Vertex, EShaderStage::Fragment },
                                               "main" });

    // Texture
    m_SkyboxTex  = CAssetManager::Get().GetOrLoadTexture("textures://HDRI/newport_loft.hdr");
    m_BRDFLutTex = CAssetManager::Get().GetOrLoadTexture("textures://brdfLUT.png");

    {
        FTextureSpecification spec;
        spec.Name   = "SkyboxCubemap";
        spec.Type   = ETextureType::TextureCube;
        spec.Size   = { 512, 512 };
        spec.Format = EPixelFormat::RGBA16F;
        spec.Usage  = ETextureUsage::Texture | ETextureUsage::Attachment;

        m_SkyboxCubemap = CreateRef<CTexture>(spec);
    }

    {
        FTextureSpecification spec;
        spec.Name   = "SkyboxIrradianceCubemap";
        spec.Type   = ETextureType::TextureCube;
        spec.Size   = { 32, 32 };
        spec.Format = EPixelFormat::RGBA16F;
        spec.Usage  = ETextureUsage::Texture | ETextureUsage::Attachment;

        m_SkyboxIrradianceCubemap = CreateRef<CTexture>(spec);
    }

    {
        FTextureSpecification spec;
        spec.Name          = "SkyboxPrefilteredCubemap";
        spec.Type          = ETextureType::TextureCube;
        spec.Size          = { 512, 512 };
        spec.Format        = EPixelFormat::RGBA16F;
        spec.Usage         = ETextureUsage::Texture | ETextureUsage::Attachment;
        spec.bGenerateMips = true;

        m_SkyboxPrefilteredCubemap = CreateRef<CTexture>(spec);
    }

    // Material
    // m_SolidMat =
    //     CAssetManager::Get().GetOrLoadMaterial({ "Solid", cubeShader, { EPixelFormat::RGBA16F }
    //     });

    m_GBufferMat = CAssetManager::Get().GetOrLoadMaterial(
        { "GBuffer",
          gBufferShader,
          { EPixelFormat::RGBA16F, EPixelFormat::RGBA16F, EPixelFormat::RGBA16F,
            EPixelFormat::RGBA16F, EPixelFormat::RGBA16F } });

    // m_PBRMat =
    //     CAssetManager::Get().GetOrLoadMaterial({ "PBR", pbrShader, { EPixelFormat::RGBA16F } });

    // Pipeline
    {

        FPipelineSpecification spec;
        spec.Name               = "CubemapSampler";
        spec.RHIShaders         = { cubemapSamplerShader->GetShaderResources() };
        spec.OutputColorFormats = { EPixelFormat::RGBA16F };
        spec.CullMode           = ECullMode::Front;
        spec.bDepthTestEnable   = false;
        spec.bDepthWriteEnable  = false;
        spec.VertexLayout       = { { EShaderDataType::Float3, "a_Position" },
                                    { EShaderDataType::Float3, "a_Normal" },
                                    { EShaderDataType::Float2, "a_TexCoord" },
                                    { EShaderDataType::Float3, "a_Tangent" },
                                    { EShaderDataType::Float3, "a_Bitangent" } };
        spec.PushConstantRanges = { { 0, sizeof(uint32_t) } };

        m_CubemapSamplerPipeline = IRHIAPI::CreatePipeline(spec);
    }

    {
        FPipelineSpecification spec;
        spec.Name               = "IrradianceConvolution";
        spec.RHIShaders         = { irradianceShader->GetShaderResources() };
        spec.OutputColorFormats = { EPixelFormat::RGBA16F };
        spec.CullMode           = ECullMode::Front;
        spec.bDepthTestEnable   = false;
        spec.bDepthWriteEnable  = false;
        spec.VertexLayout       = { { EShaderDataType::Float3, "a_Position" },
                                    { EShaderDataType::Float3, "a_Normal" },
                                    { EShaderDataType::Float2, "a_TexCoord" },
                                    { EShaderDataType::Float3, "a_Tangent" },
                                    { EShaderDataType::Float3, "a_Bitangent" } };
        spec.PushConstantRanges = { { 0, sizeof(uint32_t) } };

        m_IrradiancePipeline = IRHIAPI::CreatePipeline(spec);
    }

    {
        FPipelineSpecification spec;
        spec.Name               = "PrefilteredSampler";
        spec.RHIShaders         = { prefilteredShader->GetShaderResources() };
        spec.OutputColorFormats = { EPixelFormat::RGBA16F };
        spec.CullMode           = ECullMode::Front;
        spec.bDepthTestEnable   = false;
        spec.bDepthWriteEnable  = false;
        spec.VertexLayout       = { { EShaderDataType::Float3, "a_Position" },
                                    { EShaderDataType::Float3, "a_Normal" },
                                    { EShaderDataType::Float2, "a_TexCoord" },
                                    { EShaderDataType::Float3, "a_Tangent" },
                                    { EShaderDataType::Float3, "a_Bitangent" } };
        spec.PushConstantRanges = { { 0, sizeof(uint32_t) + sizeof(float) } };

        m_PrefilteredPipeline = IRHIAPI::CreatePipeline(spec);
    }

    {
        FPipelineSpecification spec;
        spec.Name               = "Skybox";
        spec.RHIShaders         = { skyboxShader->GetShaderResources() };
        spec.OutputColorFormats = { EPixelFormat::RGBA16F };
        spec.CullMode           = ECullMode::Front;
        spec.bDepthTestEnable   = false;
        spec.bDepthWriteEnable  = false;
        spec.VertexLayout       = { { EShaderDataType::Float3, "a_Position" },
                                    { EShaderDataType::Float3, "a_Normal" },
                                    { EShaderDataType::Float2, "a_TexCoord" },
                                    { EShaderDataType::Float3, "a_Tangent" },
                                    { EShaderDataType::Float3, "a_Bitangent" } };

        m_SkyboxPipeline = IRHIAPI::CreatePipeline(spec);
    }

    {
        FPipelineSpecification spec;
        spec.Name               = "Debug";
        spec.RHIShaders         = { debugShader->GetShaderResources() };
        spec.OutputColorFormats = { EPixelFormat::RGBA16F };
        spec.CullMode           = ECullMode::None;
        spec.bDepthTestEnable   = false;
        spec.bDepthWriteEnable  = false;
        spec.VertexLayout       = { { EShaderDataType::Float3, "a_Position" },
                                    { EShaderDataType::Float3, "a_Normal" },
                                    { EShaderDataType::Float2, "a_TexCoord" },
                                    { EShaderDataType::Float3, "a_Tangent" },
                                    { EShaderDataType::Float3, "a_Bitangent" } };

        m_DebugPipeline = IRHIAPI::CreatePipeline(spec);
    }

    {
        FPipelineSpecification spec;
        spec.Name               = "PBR";
        spec.RHIShaders         = { pbrShader->GetShaderResources() };
        spec.OutputColorFormats = { EPixelFormat::RGBA16F };
        spec.CullMode           = ECullMode::None;
        spec.bDepthTestEnable   = false;
        spec.bDepthWriteEnable  = false;
        spec.VertexLayout       = { { EShaderDataType::Float3, "a_Position" },
                                    { EShaderDataType::Float3, "a_Normal" },
                                    { EShaderDataType::Float2, "a_TexCoord" },
                                    { EShaderDataType::Float3, "a_Tangent" },
                                    { EShaderDataType::Float3, "a_Bitangent" } };

        m_PBRPipeline = IRHIAPI::CreatePipeline(spec);
    }

    // Buffers
    {
        FBufferSpecification spec;
        spec.Size       = sizeof(CameraData);
        spec.Usage      = EBufferUsage::UniformBuffer;
        spec.MemoryType = EMemoryType::HostVisible | EMemoryType::HostCoherent;
        spec.Name       = "CubemapCameraUniformBuffer";

        m_CubemapCameraBuffer = IRHIAPI::CreateBuffer(spec);
    }

    FMatrix4 view = s_CubeTextureCaptureViews[0];
    FMatrix4 proj = s_CubeTextureCaptureProjection;

    CameraData data;
    data.view       = view;
    data.projection = proj;

    FBuffer updateData(&data, sizeof(CameraData));

    m_CubemapCameraBuffer->SetData(updateData);

    m_Cube = CreateRef<FCube>();
    m_Cube->Upload();

    m_Quad = CreateRef<FQuad>();
    m_Quad->Upload();
}

void CRenderer::Tick(float deltaTime) {
    m_Profiler->Flip();

    if (m_Window->CheckAndResetVSyncDirty()) {
        EPresentMode mode =
            m_Window->IsVSyncEnabled() ? EPresentMode::FIFO : EPresentMode::Immediate;

        m_GraphicContext->GetSwapchain()->SetPresentMode(mode);
    }

    auto& cmdList    = m_Frames[m_CurrentFrameIndex].CommandList;
    auto& syncObject = m_Frames[m_CurrentFrameIndex].RenderFence;
    m_GraphicContext->SetCurrentFrame(deltaTime);
    m_GraphicContext->SetCurrentFrameIndex(m_CurrentFrameIndex);
    m_GraphicContext->GetDevice()->TickDeferredDeletion(deltaTime, m_CurrentFrameIndex);
    CCameraUniformManager::Get().UpdateAllCameras();

    IRHIAPI::DrawFrame(cmdList, syncObject, [&](uint32 imageIndex) {
        cmdList->Begin();

        // ------ Render Graph ------
        CRenderGraph graph(m_GraphicContext.get());

        FRDGTexture* skybox2DHandle = graph.ImportExternalRDGTexture(
            "SkyboxTexture", m_SkyboxTex->GetResource(), EImageLayout::ShaderReadOnlyOptimal,
            EImageLayout::ShaderReadOnlyOptimal);

        FRDGTexture* skyboxCubemapHandle = graph.ImportExternalRDGTexture(
            "SkyboxCubemap", m_SkyboxCubemap->GetResource(), EImageLayout::ColorAttachmentOptimal,
            EImageLayout::ShaderReadOnlyOptimal);

        FRDGTexture* skyboxIrradianceCubemapHandle = graph.ImportExternalRDGTexture(
            "SkyboxIrradianceCubemap", m_SkyboxIrradianceCubemap->GetResource(),
            EImageLayout::ColorAttachmentOptimal, EImageLayout::ShaderReadOnlyOptimal);

        FRDGTexture* skyboxPrefilteredCubemapHandle = graph.ImportExternalRDGTexture(
            "SkyboxPrefilteredCubemap", m_SkyboxPrefilteredCubemap->GetResource(),
            EImageLayout::ColorAttachmentOptimal, EImageLayout::ShaderReadOnlyOptimal);

        FRDGTexture* BRDFLutHandle = graph.ImportExternalRDGTexture(
            "BRDFLutTexture", m_BRDFLutTex->GetResource(), EImageLayout::ShaderReadOnlyOptimal,
            EImageLayout::ShaderReadOnlyOptimal);

        if (!m_bEnvSampled) {
            graph.AddPass("EquirectangularToCubemap", m_CubemapSamplerPipeline, { skybox2DHandle },
                          { skyboxCubemapHandle }, ERenderPassLoadOp::Clear,
                          [this, skybox2DHandle](CRDGContext& ctx) {
                              CZ_RENDERER_SCOPE_PERF(ERendererProfileSlot::CubemapSampler);
                              auto st       = ctx.GetTexture(skybox2DHandle);
                              auto cmd      = ctx.GetCommandBuffer();
                              int faceIndex = ctx.GetFaceIndex();

                              struct {
                                  uint32_t u_FaceIndex;
                              } pushConstants;
                              pushConstants.u_FaceIndex = faceIndex;
                              cmd->PushConstants(&pushConstants, sizeof(pushConstants), 0);

                              auto setLayout = m_CubemapSamplerPipeline->GetSetLayout(1);
                              std::vector<FDescriptorBinding> bindings = {
                                  { 0, EUniformType::CombinedImageSampler, st->GetImage(),
                                    st->GetSampler().get(), EImageLayout::ShaderReadOnlyOptimal }
                              };
                              auto descSet =
                                  m_GraphicContext->GetDevice()->GetOrCreateDescriptorSet(setLayout,
                                                                                          bindings);

                              uint32_t faceSize = m_SkyboxCubemap->GetSpec().Size.Width;

                              cmd->BindDescriptorSets(1, descSet);
                              cmd->SetViewport({ 0, 0, (float)faceSize, (float)faceSize, 0, 1 });
                              cmd->SetScissor({ 0, 0, faceSize, faceSize });

                              m_Quad->Draw(cmd.get());
                          });

            graph.AddPass(
                "IrradianceConvolution", m_IrradiancePipeline, { skyboxCubemapHandle },
                { skyboxIrradianceCubemapHandle }, ERenderPassLoadOp::Clear,
                [this, skyboxCubemapHandle](CRDGContext& ctx) {
                    CZ_RENDERER_SCOPE_PERF(ERendererProfileSlot::CubemapSampler);
                    auto st       = ctx.GetTexture(skyboxCubemapHandle);
                    auto cmd      = ctx.GetCommandBuffer();
                    int faceIndex = ctx.GetFaceIndex();

                    struct {
                        uint32_t u_FaceIndex;
                    } pushConstants;
                    pushConstants.u_FaceIndex = faceIndex;
                    cmd->PushConstants(&pushConstants, sizeof(pushConstants), 0);

                    {
                        auto setLayout = m_IrradiancePipeline->GetSetLayout(1);
                        std::vector<FDescriptorBinding> bindings = {
                            { 0, EUniformType::CombinedImageSampler, st->GetImage(),
                              st->GetSampler().get(), EImageLayout::ShaderReadOnlyOptimal }
                        };
                        auto descSet = m_GraphicContext->GetDevice()->GetOrCreateDescriptorSet(
                            setLayout, bindings);

                        cmd->BindDescriptorSets(1, descSet);
                    }

                    uint32_t faceSize = m_SkyboxIrradianceCubemap->GetSpec().Size.Width;

                    cmd->SetViewport({ 0, 0, (float)faceSize, (float)faceSize, 0, 1 });
                    cmd->SetScissor({ 0, 0, faceSize, faceSize });

                    m_Cube->Draw(cmd.get());
                });

            graph.AddPass(
                "Prefiltered", m_PrefilteredPipeline, { skyboxCubemapHandle },
                { skyboxPrefilteredCubemapHandle }, ERenderPassLoadOp::Clear,
                [this, skyboxCubemapHandle](CRDGContext& ctx) {
                    CZ_RENDERER_SCOPE_PERF(ERendererProfileSlot::CubemapSampler);
                    auto st       = ctx.GetTexture(skyboxCubemapHandle);
                    auto cmd      = ctx.GetCommandBuffer();
                    int faceIndex = ctx.GetFaceIndex();
                    int mip       = ctx.GetMip();

                    uint32_t faceSize = m_SkyboxPrefilteredCubemap->GetSpec().Size.Width;
                    uint32_t prefilteredMaxMip =
                        m_SkyboxPrefilteredCubemap->GetSpec().ToImageSpec().MipLevels;

                    struct {
                        uint32_t u_FaceIndex;
                        float u_Roughness;
                    } pushConstants;
                    pushConstants.u_FaceIndex = faceIndex;
                    pushConstants.u_Roughness = (float)mip / (float)(prefilteredMaxMip - 1);
                    cmd->PushConstants(&pushConstants, sizeof(pushConstants), 0);

                    {
                        auto setLayout = m_PrefilteredPipeline->GetSetLayout(1);
                        std::vector<FDescriptorBinding> bindings = {
                            { 0, EUniformType::CombinedImageSampler, st->GetImage(),
                              st->GetSampler().get(), EImageLayout::ShaderReadOnlyOptimal }
                        };
                        auto descSet = m_GraphicContext->GetDevice()->GetOrCreateDescriptorSet(
                            setLayout, bindings);

                        cmd->BindDescriptorSets(1, descSet);
                    }

                    cmd->SetViewport({ 0, 0, (float)faceSize, (float)faceSize, 0, 1 });
                    cmd->SetScissor({ 0, 0, faceSize, faceSize });

                    m_Cube->Draw(cmd.get());
                });

            m_bEnvSampled = true;
        }

        // Request a transient texture for storing the skybox pass result,
        // lifetime managed by the render graph
        for (auto& viewport : m_Viewports) {
            viewport->GetScene()->Update(deltaTime);

            IRHITexture* viewportCanvas = viewport->GetFrameBuffer()->GetColorAttachment(0).get();
            FRDGTexture* viewportHandle = graph.ImportExternalRDGTexture(
                "ViewportCanvas_" + viewport->GetName(), viewportCanvas,
                EImageLayout::ColorAttachmentOptimal, // initial usage
                EImageLayout::ShaderReadOnlyOptimal); // final usage (for UI)

            FTextureSpecification skyboxSpec;
            skyboxSpec.Type   = ETextureType::Texture2D;
            skyboxSpec.Size   = viewportCanvas->GetSize();
            skyboxSpec.Format = EPixelFormat::RGBA16F;
            skyboxSpec.Usage  = ETextureUsage::Texture | ETextureUsage::Attachment;

            FRDGTexture* skyboxHandle = graph.CreateRDGTexture("Skybox", skyboxSpec);

            graph.AddPass(
                "SkyboxPass", m_SkyboxPipeline, { skyboxCubemapHandle }, { skyboxHandle },
                ERenderPassLoadOp::Clear, [this, &viewport, skyboxCubemapHandle](CRDGContext& ctx) {
                    CZ_RENDERER_SCOPE_PERF(ERendererProfileSlot::Skybox);
                    auto st  = ctx.GetTexture(skyboxCubemapHandle);
                    auto cmd = ctx.GetCommandBuffer();

                    auto scene  = viewport->GetScene();
                    auto camera = viewport->GetCamera();
                    auto cameraBuffer =
                        CCameraUniformManager::Get().GetBufferForCamera(camera.get());
                    auto width  = viewport->GetWidth();
                    auto height = viewport->GetHeight();

                    {
                        auto setLayout = m_SkyboxPipeline->GetSetLayout(0);
                        std::vector<FDescriptorBinding> bindings = {
                            { 0, EUniformType::UniformBuffer, cameraBuffer.get(), nullptr },
                        };
                        auto descSet = m_GraphicContext->GetDevice()->GetOrCreateDescriptorSet(
                            setLayout, bindings);

                        cmd->BindDescriptorSets(0, descSet);
                    }

                    {
                        auto setLayout = m_SkyboxPipeline->GetSetLayout(1);
                        std::vector<FDescriptorBinding> bindings = {
                            { 0, EUniformType::CombinedImageSampler, st->GetImage(),
                              st->GetSampler().get(), EImageLayout::ShaderReadOnlyOptimal }
                        };
                        auto descSet = m_GraphicContext->GetDevice()->GetOrCreateDescriptorSet(
                            setLayout, bindings);

                        cmd->BindDescriptorSets(1, descSet);
                    }

                    cmd->SetViewport({ 0, 0, (float)width, (float)height, 0, 1 });
                    cmd->SetScissor({ 0, 0, width, height });

                    // cmd->Draw(4, 1, 0, 0);
                    m_Cube->Draw(cmd.get());
                });

            FTextureSpecification gBufferSpec;
            gBufferSpec.Type   = ETextureType::Texture2D;
            gBufferSpec.Size   = viewportCanvas->GetSize();
            gBufferSpec.Format = EPixelFormat::RGBA16F;
            gBufferSpec.Usage  = ETextureUsage::Texture | ETextureUsage::Attachment;

            FRDGTexture* gBufferPositionHandle =
                graph.CreateRDGTexture("GBufferPosition", gBufferSpec);
            FRDGTexture* gBufferNormalHandle = graph.CreateRDGTexture("GBufferNormal", gBufferSpec);
            FRDGTexture* gBufferBaseColorHandle =
                graph.CreateRDGTexture("GBufferBaseColor", gBufferSpec);
            FRDGTexture* gBufferRMAOHandle = graph.CreateRDGTexture("GBufferRMAO", gBufferSpec);
            FRDGTexture* gBufferEmissiveHandle =
                graph.CreateRDGTexture("GBufferEmissive", gBufferSpec);

            FTextureSpecification gBufferDepthSpec;
            gBufferDepthSpec.Type   = ETextureType::Texture2D;
            gBufferDepthSpec.Size   = viewportCanvas->GetSize();
            gBufferDepthSpec.Format = EPixelFormat::D32F;
            gBufferDepthSpec.Usage  = ETextureUsage::Texture | ETextureUsage::Attachment;
            FRDGTexture* gBufferDepthHandle =
                graph.CreateRDGTexture("GBufferDepth", gBufferDepthSpec);

            graph.AddPass("GBufferPass", nullptr, {},
                          { gBufferPositionHandle, gBufferNormalHandle, gBufferBaseColorHandle,
                            gBufferRMAOHandle, gBufferEmissiveHandle, gBufferDepthHandle },
                          ERenderPassLoadOp::Clear, [&viewport](CRDGContext& ctx) {
                              auto cmd = ctx.GetCommandBuffer();

                              auto scene  = viewport->GetScene();
                              auto camera = viewport->GetCamera();
                              auto cameraBuffer =
                                  CCameraUniformManager::Get().GetBufferForCamera(camera.get());
                              auto width  = viewport->GetWidth();
                              auto height = viewport->GetHeight();

                              cmd->SetViewport({ 0, 0, (float)width, (float)height, 0, 1 });
                              cmd->SetScissor({ 0, 0, width, height });

                              scene->Draw(cmd.get(), cameraBuffer);
                          });

            FTextureSpecification pbrSpec;
            pbrSpec.Type   = ETextureType::Texture2D;
            pbrSpec.Size   = viewportCanvas->GetSize();
            pbrSpec.Format = EPixelFormat::RGBA16F;
            pbrSpec.Usage  = ETextureUsage::Texture | ETextureUsage::Attachment;

            FRDGTexture* pbrHandle = graph.CreateRDGTexture("PBR", pbrSpec);

            graph.AddPass(
                "PBRPass", m_PBRPipeline,
                { gBufferPositionHandle, gBufferNormalHandle, gBufferBaseColorHandle,
                  gBufferRMAOHandle, gBufferEmissiveHandle, gBufferDepthHandle, BRDFLutHandle,
                  skyboxIrradianceCubemapHandle, skyboxPrefilteredCubemapHandle },
                { pbrHandle }, ERenderPassLoadOp::Load,
                [this, &viewport, gBufferPositionHandle, gBufferNormalHandle,
                 gBufferBaseColorHandle, gBufferRMAOHandle, gBufferEmissiveHandle,
                 gBufferDepthHandle, BRDFLutHandle, skyboxIrradianceCubemapHandle,
                 skyboxPrefilteredCubemapHandle](CRDGContext& ctx) {
                    auto t1 = ctx.GetTexture(gBufferPositionHandle);
                    auto t2 = ctx.GetTexture(gBufferNormalHandle);
                    auto t3 = ctx.GetTexture(gBufferBaseColorHandle);
                    auto t4 = ctx.GetTexture(gBufferRMAOHandle);
                    auto t5 = ctx.GetTexture(gBufferEmissiveHandle);
                    auto t6 = ctx.GetTexture(gBufferDepthHandle);
                    auto t7 = ctx.GetTexture(BRDFLutHandle);
                    auto t8 = ctx.GetTexture(skyboxIrradianceCubemapHandle);
                    auto t9 = ctx.GetTexture(skyboxPrefilteredCubemapHandle);

                    auto cmd = ctx.GetCommandBuffer();

                    auto camera = viewport->GetCamera();
                    auto cameraBuffer =
                        CCameraUniformManager::Get().GetBufferForCamera(camera.get());
                    auto width  = viewport->GetWidth();
                    auto height = viewport->GetHeight();

                    {
                        auto setLayout                           = m_PBRPipeline->GetSetLayout(0);
                        std::vector<FDescriptorBinding> bindings = {
                            { 0, EUniformType::UniformBuffer, cameraBuffer.get(), nullptr },
                        };
                        auto descSet = m_GraphicContext->GetDevice()->GetOrCreateDescriptorSet(
                            setLayout, bindings);

                        cmd->BindDescriptorSets(0, descSet);
                    }

                    {
                        struct alignas(16) SceneUBO {
                            FVector3 CameraPosition;
                            float _padding;
                        };

                        SceneUBO uniforms;
                        uniforms.CameraPosition = camera->GetPosition();

                        FBufferSpecification bufferSpec;
                        bufferSpec.Usage = EBufferUsage::UniformBuffer;
                        bufferSpec.Size  = sizeof(SceneUBO);
                        bufferSpec.MemoryType =
                            EMemoryType::HostVisible | EMemoryType::HostCoherent;
                        bufferSpec.Name = "SceneUniformBuffer";

                        FBuffer data(&uniforms, bufferSpec.Size);

                        if (m_SceneUniformBuffer) {
                            m_SceneUniformBuffer->SetData(data);
                        } else {
                            m_SceneUniformBuffer = IRHIAPI::CreateBuffer(bufferSpec, data);
                        }

                        auto setLayout                           = m_PBRPipeline->GetSetLayout(1);
                        std::vector<FDescriptorBinding> bindings = {
                            { 0, EUniformType::UniformBuffer, m_SceneUniformBuffer.get(), nullptr },
                            { 1, EUniformType::CombinedImageSampler, t1->GetImage(),
                              t1->GetSampler().get(), EImageLayout::ShaderReadOnlyOptimal },
                            { 2, EUniformType::CombinedImageSampler, t2->GetImage(),
                              t2->GetSampler().get(), EImageLayout::ShaderReadOnlyOptimal },
                            { 3, EUniformType::CombinedImageSampler, t3->GetImage(),
                              t3->GetSampler().get(), EImageLayout::ShaderReadOnlyOptimal },
                            { 4, EUniformType::CombinedImageSampler, t4->GetImage(),
                              t4->GetSampler().get(), EImageLayout::ShaderReadOnlyOptimal },
                            { 5, EUniformType::CombinedImageSampler, t5->GetImage(),
                              t5->GetSampler().get(), EImageLayout::ShaderReadOnlyOptimal },
                            { 6, EUniformType::CombinedImageSampler, t6->GetImage(),
                              t6->GetSampler().get(), EImageLayout::ShaderReadOnlyOptimal },
                            { 7, EUniformType::CombinedImageSampler, t7->GetImage(),
                              t7->GetSampler().get(), EImageLayout::ShaderReadOnlyOptimal },
                            { 8, EUniformType::CombinedImageSampler, t8->GetImage(),
                              t8->GetSampler().get(), EImageLayout::ShaderReadOnlyOptimal },
                            { 9, EUniformType::CombinedImageSampler, t9->GetImage(),
                              t9->GetSampler().get(), EImageLayout::ShaderReadOnlyOptimal }
                        };
                        auto descSet = m_GraphicContext->GetDevice()->GetOrCreateDescriptorSet(
                            setLayout, bindings);

                        cmd->BindDescriptorSets(1, descSet);
                    }

                    cmd->SetViewport({ 0, 0, (float)width, (float)height, 0, 1 });
                    cmd->SetScissor({ 0, 0, width, height });

                    cmd->Draw(3, 1, 0, 0);
                });

            graph.AddPass(
                "DebugPass", m_DebugPipeline,
                { pbrHandle, gBufferPositionHandle, gBufferNormalHandle, gBufferBaseColorHandle,
                  gBufferRMAOHandle, gBufferEmissiveHandle, gBufferDepthHandle, skyboxHandle,
                  skyboxCubemapHandle, skyboxIrradianceCubemapHandle,
                  skyboxPrefilteredCubemapHandle },
                { viewportHandle }, ERenderPassLoadOp::Load,
                [this, &viewport, pbrHandle, gBufferPositionHandle, gBufferNormalHandle,
                 gBufferBaseColorHandle, gBufferRMAOHandle, gBufferEmissiveHandle,
                 gBufferDepthHandle, skyboxHandle, skyboxCubemapHandle,
                 skyboxIrradianceCubemapHandle, skyboxPrefilteredCubemapHandle](CRDGContext& ctx) {
                    auto t1  = ctx.GetTexture(pbrHandle);
                    auto t2  = ctx.GetTexture(gBufferPositionHandle);
                    auto t3  = ctx.GetTexture(gBufferNormalHandle);
                    auto t4  = ctx.GetTexture(gBufferBaseColorHandle);
                    auto t5  = ctx.GetTexture(gBufferRMAOHandle);
                    auto t6  = ctx.GetTexture(gBufferEmissiveHandle);
                    auto t7  = ctx.GetTexture(gBufferDepthHandle);
                    auto t8  = ctx.GetTexture(skyboxHandle);
                    auto t9  = ctx.GetTexture(skyboxCubemapHandle);
                    auto t10 = ctx.GetTexture(skyboxIrradianceCubemapHandle);
                    auto t11 = ctx.GetTexture(skyboxPrefilteredCubemapHandle);

                    auto cmd = ctx.GetCommandBuffer();

                    auto width  = viewport->GetWidth();
                    auto height = viewport->GetHeight();

                    {
                        struct alignas(16) DebugUBO {
                            int Mode;
                            int padding[3];
                        };

                        DebugUBO uniforms;
                        uniforms.Mode = m_DebugMode;

                        FBufferSpecification bufferSpec;
                        bufferSpec.Usage = EBufferUsage::UniformBuffer;
                        bufferSpec.Size  = sizeof(DebugUBO);
                        bufferSpec.MemoryType =
                            EMemoryType::HostVisible | EMemoryType::HostCoherent;
                        bufferSpec.Name = "DebugUniformBuffer";

                        FBuffer data(&uniforms, bufferSpec.Size);

                        if (m_DebugUniformBuffer) {
                            m_DebugUniformBuffer->SetData(data);
                        } else {
                            m_DebugUniformBuffer = IRHIAPI::CreateBuffer(bufferSpec, data);
                        }

                        auto setLayout                           = m_DebugPipeline->GetSetLayout(1);
                        std::vector<FDescriptorBinding> bindings = {
                            { 0, EUniformType::UniformBuffer, m_DebugUniformBuffer.get(), nullptr },
                            { 1, EUniformType::CombinedImageSampler, t1->GetImage(),
                              t1->GetSampler().get(), EImageLayout::ShaderReadOnlyOptimal },
                            { 2, EUniformType::CombinedImageSampler, t2->GetImage(),
                              t2->GetSampler().get(), EImageLayout::ShaderReadOnlyOptimal },
                            { 3, EUniformType::CombinedImageSampler, t3->GetImage(),
                              t3->GetSampler().get(), EImageLayout::ShaderReadOnlyOptimal },
                            { 4, EUniformType::CombinedImageSampler, t4->GetImage(),
                              t4->GetSampler().get(), EImageLayout::ShaderReadOnlyOptimal },
                            { 5, EUniformType::CombinedImageSampler, t5->GetImage(),
                              t5->GetSampler().get(), EImageLayout::ShaderReadOnlyOptimal },
                            { 6, EUniformType::CombinedImageSampler, t6->GetImage(),
                              t6->GetSampler().get(), EImageLayout::ShaderReadOnlyOptimal },
                            { 7, EUniformType::CombinedImageSampler, t7->GetImage(),
                              t7->GetSampler().get(), EImageLayout::ShaderReadOnlyOptimal },
                            { 8, EUniformType::CombinedImageSampler, t8->GetImage(),
                              t8->GetSampler().get(), EImageLayout::ShaderReadOnlyOptimal },
                            { 9, EUniformType::CombinedImageSampler, t9->GetImage(),
                              t9->GetSampler().get(), EImageLayout::ShaderReadOnlyOptimal },
                            { 10, EUniformType::CombinedImageSampler, t10->GetImage(),
                              t10->GetSampler().get(), EImageLayout::ShaderReadOnlyOptimal },
                            { 11, EUniformType::CombinedImageSampler, t11->GetImage(),
                              t11->GetSampler().get(), EImageLayout::ShaderReadOnlyOptimal },

                        };
                        auto descSet = m_GraphicContext->GetDevice()->GetOrCreateDescriptorSet(
                            setLayout, bindings);

                        cmd->BindDescriptorSets(1, descSet);
                    }

                    cmd->SetViewport({ 0, 0, (float)width, (float)height, 0, 1 });
                    cmd->SetScissor({ 0, 0, width, height });

                    cmd->Draw(3, 1, 0, 0);
                });

            // graph.AddPass("SceneCompositePass", nullptr, {}, { viewportHandle },
            //               ERenderPassLoadOp::Load, [this, &viewport](CRDGContext& ctx) {
            //                   CZ_RENDERER_SCOPE_PERF(ERendererProfileSlot::Composite);

            //                   auto cmd = ctx.GetCommandBuffer();

            //                   auto scene  = viewport->GetScene();
            //                   auto camera = viewport->GetCamera();
            //                   auto cameraBuffer =
            //                       CCameraUniformManager::Get().GetBufferForCamera(camera.get());
            //                   auto width  = viewport->GetWidth();
            //                   auto height = viewport->GetHeight();

            //                   cmd->SetViewport({ 0, 0, (float)width, (float)height, 0, 1 });
            //                   cmd->SetScissor({ 0, 0, width, height });

            //                   scene->Draw(cmd.get(), cameraBuffer);
            //               });
        }

        // // draw UI on top of the scene
        IRHITexture* backbuffer =
            m_GraphicContext->GetSwapchain()->GetColorAttachment(imageIndex).get();
        // auto tex = CreateRef<CTexture>(backbuffer->GetSpec(), backbuffer);

        FRDGTexture* BackbufferHandle = graph.ImportExternalRDGTexture(
            "Backbuffer", backbuffer, EImageLayout::ColorAttachmentOptimal, // initial usage
            EImageLayout::PresentSrc);

        graph.AddPass("SwapchainPass", nullptr, {}, { BackbufferHandle }, ERenderPassLoadOp::Clear,
                      [this](CRDGContext& ctx) {
                          CZ_RENDERER_SCOPE_PERF(ERendererProfileSlot::ImGUI);

                          if (m_UICallback) {
                              m_UICallback(ctx.GetCommandBuffer());
                          }
                      });

        {
            CZ_RENDERER_SCOPE_PERF(ERendererProfileSlot::GraphCompile);
            graph.Compile(); // Request physical resources from the global singleton ImagePool
        }

        {
            CZ_RENDERER_SCOPE_PERF(ERendererProfileSlot::GraphExecute);
            graph.Execute(cmdList.get());
        }

        // ------ Render Graph End ------

        cmdList->End();
    });

    m_CurrentFrameIndex = (m_CurrentFrameIndex + 1) % m_GraphicContext->GetMaxFramesInFlight();
}

void CRenderer::Shutdown() {
    if (!m_GraphicContext) return;

    m_GraphicContext->GetDevice()->WaitIdle();

    m_SkyboxTex.Reset();     // TODO: Remove
    m_SkyboxCubemap.Reset(); // TODO: Remove
    m_BRDFLutTex.Reset();
    m_SkyboxIrradianceCubemap.Reset();
    m_SkyboxPrefilteredCubemap.Reset();
    m_Cube.Reset();
    m_Quad.Reset();

    m_Viewports.clear();

    m_CubemapSamplerPipeline.Reset();
    m_IrradiancePipeline.Reset();
    m_PrefilteredPipeline.Reset();
    m_SkyboxPipeline.Reset();
    m_DebugPipeline.Reset();
    m_PBRPipeline.Reset();

    m_DebugUniformBuffer.Reset();
    m_CubemapCameraBuffer.Reset();
    m_SceneUniformBuffer.Reset();

    m_SolidMat.Reset();
    m_GBufferMat.Reset();
    m_PBRMat.Reset();

    for (int i = 0; i < m_GraphicContext->GetMaxFramesInFlight(); i++) {
        m_Frames[i].RenderFence.Reset();
        m_Frames[i].CommandList.Reset();
        m_Frames[i].CommandPool.Reset();
    }

    CCameraUniformManager::Get().Shutdown();
    CMeshManager::Get().Shutdown();
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
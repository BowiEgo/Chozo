#include "SceneRenderer.h"

#include "Renderer.h"
#include "RenderCommand.h"

namespace Chozo
{

    static std::vector<Ref<SceneRenderer>> s_SceneRenderers;

    SceneRenderer::SceneRenderer(Ref<Scene>& scene)
        : m_Scene(scene)
    {
        Init();
    }

    void SceneRenderer::Init()
    {
        m_CommandBuffer = RenderCommandBuffer::Create();

        // Uniform buffers
        m_CameraUB = UniformBuffer::Create(sizeof(CameraData));
        m_SceneUB = UniformBuffer::Create(sizeof(SceneData));
        m_PointLightUB = UniformBuffer::Create(sizeof(PointLightsData));
        m_SpotLightUB = UniformBuffer::Create(sizeof(SpotLightsData));

        // Skybox
        {
			auto skyboxShader = Renderer::GetShaderLibrary()->Get("Skybox");

			FramebufferSpecification fbSpec;
			fbSpec.Attachments = { ImageFormat::RGBA32F };
			// fbSpec.ExistingImages[0] = m_CompositePass->GetOutput(0);
			Ref<Framebuffer> framebuffer = Framebuffer::Create(fbSpec);
            
			PipelineSpecification pipelineSpec;
			pipelineSpec.DebugName = "Skybox";
			pipelineSpec.Shader = skyboxShader;
            pipelineSpec.DepthWrite = false;
			pipelineSpec.DepthTest = false;
            pipelineSpec.Layout = {
				{ ShaderDataType::Float3, "a_Position" },
			};
			pipelineSpec.TargetFramebuffer = framebuffer;
			m_SkyboxMaterial = Material::Create(pipelineSpec.Shader, pipelineSpec.DebugName);

			RenderPassSpecification renderPassSpec;
			renderPassSpec.DebugName = "Skybox";
			renderPassSpec.Pipeline = Pipeline::Create(pipelineSpec);
			m_SkyboxPass = RenderPass::Create(renderPassSpec);
			m_SkyboxPass->SetInput("CameraData", m_CameraUB);
			// m_SkyboxPass->Bake();
        }

        // G-Buffer
        {
            // auto skyboxShader = Renderer::GetShaderLibrary()->Get("Geometry");

			FramebufferSpecification fbSpec;
			// fbSpec.Attachments = { ImageFormat::RGB16F, ImageFormat::RGB16F, ImageFormat::RGB16F, ImageFormat::RGB16F, ImageFormat::RGB16F, ImageFormat::RGB16F, ImageFormat::RGB16F, ImageFormat::RED32I, ImageFormat::Depth };
			fbSpec.Attachments = {
                ImageFormat::RGB16F,
                ImageFormat::RGB16F,
                ImageFormat::RGB16F,
                ImageFormat::RGB16F,
                ImageFormat::RGB16F,
                ImageFormat::RGB16F,
                ImageFormat::RGB16F,
                ImageFormat::RGB16F,
                // ImageFormat::RED32I,
                ImageFormat::Depth
            };
			// fbSpec.ExistingImages[0] = m_CompositePass->GetOutput(0);
			Ref<Framebuffer> framebuffer = Framebuffer::Create(fbSpec);

			PipelineSpecification pipelineSpec;
			pipelineSpec.DebugName = "Geometry";
			// pipelineSpec.Shader = skyboxShader;
			// pipelineSpec.DepthOperator = DepthCompareOperator::Equal;
            pipelineSpec.DepthWrite = false;
            // pipelineSpec.Layout = {
			// 	{ ShaderDataType::Float3, "a_Position" },
			// };
            // pipelineSpec.Shader = Renderer::GetShaderLibrary()->Get("Depth");
			pipelineSpec.TargetFramebuffer = framebuffer;

			RenderPassSpecification renderPassSpec;
			renderPassSpec.DebugName = "Geometry";
			renderPassSpec.Pipeline = Pipeline::Create(pipelineSpec);
			m_GeometryPass = RenderPass::Create(renderPassSpec);
			m_GeometryPass->SetInput("CameraData", m_CameraUB);
        }

        // Phong-Light
        {
            FramebufferSpecification fbSpec;
			fbSpec.Attachments = { ImageFormat::RGBA32F };
			// fbSpec.ExistingImages[0] = m_CompositePass->GetOutput(0);
			Ref<Framebuffer> framebuffer = Framebuffer::Create(fbSpec);

			PipelineSpecification pipelineSpec;
			pipelineSpec.DebugName = "PhongLight";
            pipelineSpec.DepthWrite = false;
			pipelineSpec.TargetFramebuffer = framebuffer;
            pipelineSpec.Shader = Renderer::GetShaderLibrary()->Get("PhongLight");
			m_PhongLightMaterial = Material::Create(pipelineSpec.Shader, pipelineSpec.DebugName);
            Ref<Texture2D> positionTex = m_GeometryPass->GetOutput(0);
            Ref<Texture2D> normalTex = m_GeometryPass->GetOutput(1);
            Ref<Texture2D> ambientTex = m_GeometryPass->GetOutput(3);
            Ref<Texture2D> diffuseTex = m_GeometryPass->GetOutput(4);
            Ref<Texture2D> specularTex = m_GeometryPass->GetOutput(5);
            Ref<Texture2D> metalnessTex = m_GeometryPass->GetOutput(6);
            Ref<Texture2D> roughnessTex = m_GeometryPass->GetOutput(7);
            m_PhongLightMaterial->Set("u_PositionTex", positionTex);
            m_PhongLightMaterial->Set("u_NormalTex", normalTex);
            m_PhongLightMaterial->Set("u_AmbientTex", ambientTex);
            m_PhongLightMaterial->Set("u_DiffuseTex", diffuseTex);
            m_PhongLightMaterial->Set("u_SpecularTex", specularTex);
            m_PhongLightMaterial->Set("u_MetalnessTex", metalnessTex);
            m_PhongLightMaterial->Set("u_RoughnessTex", roughnessTex);

			RenderPassSpecification renderPassSpec;
			renderPassSpec.DebugName = "PhongLight";
			renderPassSpec.Pipeline = Pipeline::Create(pipelineSpec);
			m_PhongLightPass = RenderPass::Create(renderPassSpec);
			m_PhongLightPass->SetInput("SceneData", m_SceneUB);
			m_PhongLightPass->SetInput("PointLightData", m_PointLightUB);
			m_PhongLightPass->SetInput("SpotLightData", m_SpotLightUB);
        }

        // PBR-PreRender
        {
            // FramebufferSpecification fbSpec;
			// fbSpec.Attachments = { ImageFormat::RGBA32F };
			// Ref<Framebuffer> framebuffer = Framebuffer::Create(fbSpec);

            // PipelineSpecification irradiancePipelineSpec;
			// irradiancePipelineSpec.DebugName = "PBR-Irradiance";
			// irradiancePipelineSpec.TargetFramebuffer = framebuffer;
            // irradiancePipelineSpec.Shader = Renderer::GetShaderLibrary()->Get("IrradianceConvolution");
			// m_PBRIrradianceMaterial = Material::Create(irradiancePipelineSpec.Shader, irradiancePipelineSpec.DebugName);

            // RenderPassSpecification irradianceRenderPassSpec;
			// irradianceRenderPassSpec.DebugName = "PhongLight";
			// irradianceRenderPassSpec.Pipeline = Pipeline::Create(irradiancePipelineSpec);
			// m_PBRIrradiancePass = RenderPass::Create(irradianceRenderPassSpec);
        }

        // PBR
        {
            FramebufferSpecification fbSpec;
			fbSpec.Attachments = { ImageFormat::RGBA32F };
			// fbSpec.ExistingImages[0] = m_CompositePass->GetOutput(0);
			Ref<Framebuffer> framebuffer = Framebuffer::Create(fbSpec);

			PipelineSpecification pipelineSpec;
			pipelineSpec.DebugName = "PBR";
            pipelineSpec.DepthWrite = false;
			pipelineSpec.TargetFramebuffer = framebuffer;
            pipelineSpec.Shader = Renderer::GetShaderLibrary()->Get("PBR");
			m_PBRMaterial = Material::Create(pipelineSpec.Shader, pipelineSpec.DebugName);

			RenderPassSpecification renderPassSpec;
			renderPassSpec.DebugName = "PBR";
			renderPassSpec.Pipeline = Pipeline::Create(pipelineSpec);
			m_PBRPass = RenderPass::Create(renderPassSpec);
			m_PBRPass->SetInput("SceneData", m_SceneUB);
			m_PBRPass->SetInput("PointLightData", m_PointLightUB);
			m_PBRPass->SetInput("SpotLightData", m_SpotLightUB);

            TextureCubeSpecification irrandianceMapSpec;
            irrandianceMapSpec.Width = 32;
            irrandianceMapSpec.Height = 32;
            irrandianceMapSpec.Format = ImageFormat::RGB16F;
            irrandianceMapSpec.WrapR = ImageParameter::CLAMP_TO_BORDER;
            irrandianceMapSpec.WrapS = ImageParameter::CLAMP_TO_BORDER;
            irrandianceMapSpec.WrapT = ImageParameter::CLAMP_TO_BORDER;
            m_PBRIrradiance = TextureCube::Create(irrandianceMapSpec);
        }

        // Final composite
        {
            FramebufferSpecification fbSpec;
            fbSpec.ClearColor = { 0.5f, 0.1f, 0.1f, 1.0f };
            fbSpec.Attachments = { ImageFormat::RGBA32F, ImageFormat::Depth };
            Ref<Framebuffer> framebuffer = Framebuffer::Create(fbSpec);

            PipelineSpecification pipelineSpec;
            pipelineSpec.Layout = {
                { ShaderDataType::Float3, "a_Position" },
                { ShaderDataType::Float2, "a_TexCoord" }
            };
            pipelineSpec.BackfaceCulling = false;
            pipelineSpec.Shader = Renderer::GetShaderLibrary()->Get("SceneComposite");
            pipelineSpec.TargetFramebuffer = framebuffer;
            pipelineSpec.DebugName = "SceneComposite";
            pipelineSpec.DepthWrite = false;
            pipelineSpec.DepthTest = false;
			m_CompositeMaterial = Material::Create(pipelineSpec.Shader, pipelineSpec.DebugName);
            Ref<Texture2D> skyboxTex = m_SkyboxPass->GetOutput(0);
            Ref<Texture2D> phongLightTex = m_PhongLightPass->GetOutput(0);
            Ref<Texture2D> PBRTex = m_PBRPass->GetOutput(0);
            Ref<Texture2D> depthText = m_GeometryPass->GetOutput(2);
            m_CompositeMaterial->Set("u_SkyboxTex", skyboxTex);
            m_CompositeMaterial->Set("u_GeometryTex", PBRTex);
            m_CompositeMaterial->Set("u_DepthTex", depthText);

            RenderPassSpecification renderPassSpec;
            renderPassSpec.DebugName = "SceneComposite";
            renderPassSpec.Pipeline = Pipeline::Create(pipelineSpec);
            m_CompositePass = RenderPass::Create(renderPassSpec);
        }
    }

    void SceneRenderer::Shutdown()
    {
    }

    void SceneRenderer::SetScene(Ref<Scene>& scene)
    {
        m_Scene = scene;
    }

    void SceneRenderer::BeginScene(EditorCamera& camera)
    {
        m_CompositePass->GetTargetFramebuffer()->Resize(m_ViewportWidth, m_ViewportHeight);
        m_SkyboxPass->GetTargetFramebuffer()->Resize(m_ViewportWidth, m_ViewportHeight);
        m_GeometryPass->GetTargetFramebuffer()->Resize(m_ViewportWidth, m_ViewportHeight);
        m_PhongLightPass->GetTargetFramebuffer()->Resize(m_ViewportWidth, m_ViewportHeight);
        m_PBRPass->GetTargetFramebuffer()->Resize(m_ViewportWidth, m_ViewportHeight);

        m_SceneData.SceneCamera = camera;
		m_SceneData.SceneEnvironment = m_Scene->m_Environment;
		m_SceneData.SceneEnvironmentIntensity = m_Scene->m_EnvironmentIntensity;
		m_SceneData.SkyboxLod = m_Scene->m_SkyboxLod;

        CameraDataUB.ProjectionMatrix = camera.GetProjection();
        CameraDataUB.ViewMatrix = camera.GetViewMatrix();

        SceneDataUB.CameraPosition = camera.GetPosition();
        SceneDataUB.EnvironmentMapIntensity = m_Scene->m_EnvironmentIntensity;

        m_CameraUB->SetData(&CameraDataUB, sizeof(CameraData));
        m_SceneUB->SetData(&SceneDataUB, sizeof(SceneData));
        m_PointLightUB->SetData(&PointLightsDataUB, sizeof(PointLightsData));
        m_SpotLightUB->SetData(&SpotLightsDataUB, sizeof(SpotLightsData));
        PointLightsDataUB.LightCount = 0;
        SpotLightsDataUB.LightCount = 0;
    }

    void SceneRenderer::EndScene()
    {
        Renderer::RenderStaticBatches();
        Flush();
    }

    void SceneRenderer::SetViewportSize(uint32_t width, uint32_t height)
    {
        if (m_ViewportWidth != width || m_ViewportHeight != height)
		{
			m_ViewportWidth = width;
			m_ViewportHeight = height;
		}
    }

    bool SceneRenderer::SubmitDirectionalLight(DirectionalLightComponent *light)
    {
        SceneDataUB.Lights.Direction = light->Direction;
        SceneDataUB.Lights.Color = light->Color;
        SceneDataUB.Lights.Intensity = light->Intensity;

        return true;
    }

    bool SceneRenderer::SubmitPointLight(PointLightComponent *light, glm::vec3 &position)
    {
        uint index = PointLightsDataUB.LightCount;
        if (index >= 1000)
        {
            CZ_CORE_WARN("PointLightBuffer is full, cannot submit more point lights.");
            return false;
        }

        PointLightsDataUB.Lights[index].Position = position;
        PointLightsDataUB.Lights[index].Intensity = light->Intensity;
        PointLightsDataUB.Lights[index].Color = light->Color;

        PointLightsDataUB.LightCount++;

        return true;
    }

    bool SceneRenderer::SubmitSpotLight(SpotLightComponent *light, glm::vec3 &position)
    {
        uint index = SpotLightsDataUB.LightCount;
        if (index >= 1000)
        {
            CZ_CORE_WARN("SpotLightBuffer is full, cannot submit more point lights.");
            return false;
        }

        SpotLightsDataUB.Lights[index].Position = position;
        SpotLightsDataUB.Lights[index].Intensity = light->Intensity;
        SpotLightsDataUB.Lights[index].Direction = light->Direction;
        SpotLightsDataUB.Lights[index].AngleAttenuation = light->AngleAttenuation;
        SpotLightsDataUB.Lights[index].Color = light->Color;
        SpotLightsDataUB.Lights[index].Angle = light->Angle;

        SpotLightsDataUB.LightCount++;

        return true;
    }

    void SceneRenderer::SubmitMesh(Ref<DynamicMesh> mesh, Ref<Material> material, const glm::mat4& transform)
    {
        MeshData meshData;
        meshData.Mesh = mesh;
        meshData.Material = material;
        meshData.Transform = transform;
        m_MeshDatas.push_back(meshData);
    }

    void SceneRenderer::SkyboxPass()
    {
		Renderer::BeginRenderPass(m_CommandBuffer, m_SkyboxPass);

        m_SkyboxMaterial->Set("u_FragUniforms.TextureLod", m_SceneData.SkyboxLod);
		m_SkyboxMaterial->Set("u_FragUniforms.Intensity", m_SceneData.SceneEnvironmentIntensity);

		const Ref<TextureCube> radianceMap = m_SceneData.SceneEnvironment ? m_SceneData.SceneEnvironment->RadianceMap : Renderer::GetBlackTextureCube();
		m_SkyboxMaterial->Set("u_Texture", radianceMap);

		Renderer::SubmitFullscreenBox(m_CommandBuffer, m_SkyboxPass->GetPipeline(), m_SkyboxMaterial);

		Renderer::EndRenderPass(m_CommandBuffer, m_SkyboxPass);
    }

    void SceneRenderer::GeometryPass()
    {
		Renderer::BeginRenderPass(m_CommandBuffer, m_GeometryPass);
        for (auto meshData : m_MeshDatas)
        {
            meshData.Material->Set("u_VertUniforms.ModelMatrix", meshData.Transform);
            
            if (static_cast<DynamicMesh*>(meshData.Mesh.get()))
                Renderer::SubmitMeshWithMaterial(m_CommandBuffer, m_GeometryPass->GetPipeline(), std::dynamic_pointer_cast<DynamicMesh>(meshData.Mesh), meshData.Material);
        }
		Renderer::EndRenderPass(m_CommandBuffer, m_GeometryPass);
    }

    void SceneRenderer::PhongLightPass()
    {
        Renderer::BeginRenderPass(m_CommandBuffer, m_PhongLightPass);
		Renderer::SubmitFullscreenQuad(m_CommandBuffer, m_PhongLightPass->GetPipeline(), m_PhongLightMaterial);
		Renderer::EndRenderPass(m_CommandBuffer, m_PhongLightPass);
    }

    void SceneRenderer::PBRPrePass()
    {
        // m_PBRIrradiancePass->GetTargetFramebuffer()->Resize(32, 32);
		// Renderer::BeginRenderPass(m_CommandBuffer, m_PBRIrradiancePass);

		// const Ref<TextureCube> radianceMap = m_SceneData.SceneEnvironment ? m_SceneData.SceneEnvironment->RadianceMap : Renderer::GetBlackTextureCube();
		// m_PBRIrradianceMaterial->Set("u_Texture", radianceMap);
        
		// Renderer::SubmitCubeMap(m_CommandBuffer, m_PBRIrradiancePass->GetPipeline(), m_PBRIrradiance, m_PBRIrradianceMaterial);
		// Renderer::EndRenderPass(m_CommandBuffer, m_PBRIrradiancePass);
    }

    void SceneRenderer::PBRPass()
    {
        Renderer::BeginRenderPass(m_CommandBuffer, m_PBRPass);
        Ref<Texture2D> positionTex = m_GeometryPass->GetOutput(0);
        Ref<Texture2D> normalTex = m_GeometryPass->GetOutput(1);
        Ref<Texture2D> ambientTex = m_GeometryPass->GetOutput(3);
        Ref<Texture2D> diffuseTex = m_GeometryPass->GetOutput(4);
        Ref<Texture2D> specularTex = m_GeometryPass->GetOutput(5);
        Ref<Texture2D> metalnessTex = m_GeometryPass->GetOutput(6);
        Ref<Texture2D> roughnessTex = m_GeometryPass->GetOutput(7);
        Ref<TextureCube> irradianceMap = Renderer::GetIrradianceTextureCube();
        Ref<TextureCube> prefilterMap = Renderer::GetPrefilteredTextureCube();
        Ref<Texture2D> brdfLUTTexture = Renderer::GetBRDFLutTexture();
        m_PBRMaterial->Set("u_PositionTex", positionTex);
        m_PBRMaterial->Set("u_NormalTex", normalTex);
        m_PBRMaterial->Set("u_DiffuseTex", diffuseTex);
        m_PBRMaterial->Set("u_MetalnessTex", metalnessTex);
        m_PBRMaterial->Set("u_RoughnessTex", roughnessTex);
        m_PBRMaterial->Set("u_IrradianceMap", irradianceMap);
        m_PBRMaterial->Set("u_PrefilterMap", prefilterMap);
        m_PBRMaterial->Set("u_BRDFLutTex", brdfLUTTexture);
		Renderer::SubmitFullscreenQuad(m_CommandBuffer, m_PBRPass->GetPipeline(), m_PBRMaterial);
		Renderer::EndRenderPass(m_CommandBuffer, m_PBRPass);
    }

    void SceneRenderer::CompositePass()
    {
		Renderer::BeginRenderPass(m_CommandBuffer, m_CompositePass);
		Renderer::SubmitFullscreenQuad(m_CommandBuffer, m_CompositePass->GetPipeline(), m_CompositeMaterial);
		Renderer::EndRenderPass(m_CommandBuffer, m_CompositePass);
    }

    void SceneRenderer::Flush()
    {
        m_CommandBuffer->Begin();

        SkyboxPass();
        GeometryPass();
        // PhongLightPass();
        PBRPass();
        CompositePass();

        m_CommandBuffer->End();

        m_MeshDatas.clear();
    }

    Ref<SceneRenderer> SceneRenderer::Create(Ref<Scene>& scene)
    {
        Ref<SceneRenderer> sceneRenderer = Ref<SceneRenderer>(new SceneRenderer(scene));
        s_SceneRenderers.push_back(sceneRenderer);

        return sceneRenderer;
    }

    Ref<SceneRenderer> SceneRenderer::Find(Scene* scene)
    {
        for (const auto& sceneRenderer : s_SceneRenderers) {
            if (sceneRenderer->GetScene().get() == scene && sceneRenderer->IsActive())
                return sceneRenderer;
        }
        return nullptr;
    }
}

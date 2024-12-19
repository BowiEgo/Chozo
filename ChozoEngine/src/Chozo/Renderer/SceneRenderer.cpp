#include "SceneRenderer.h"

#include "Renderer.h"
#include "RenderCommand.h"

namespace Chozo
{

    SceneRenderer::SceneRenderer(Ref<Scene> scene)
        : m_Scene(scene)
    {
        Init();
    }

    SceneRenderer::~SceneRenderer()
    {
        Shutdown();
    }

    void SceneRenderer::Init()
    {
        m_CommandBuffer = RenderCommandBuffer::Create();

        // Uniform buffers
        m_CameraUB = UniformBuffer::Create(sizeof(CameraData));
        m_SceneUB = UniformBuffer::Create(sizeof(SceneData));
        m_DirectionalLightsUB = UniformBuffer::Create(sizeof(DirectionalLightsData));
        m_PointLightsUB = UniformBuffer::Create(sizeof(PointLightsData));
        m_SpotLightsUB = UniformBuffer::Create(sizeof(SpotLightsData));

        // Skybox
        {
			auto skyboxShader = Renderer::GetShaderLibrary()->Get("Skybox");

			FramebufferSpecification fbSpec;
            fbSpec.ClearColor = { 0.0f, 0.0f, 0.0f, 0.0f };
			fbSpec.Attachments = { ImageFormat::RGBA16F };
			// fbSpec.ExistingImages[0] = m_CompositePass->GetOutput(0);
            
			PipelineSpecification pipelineSpec;
			pipelineSpec.DebugName = "Skybox";
			pipelineSpec.Shader = skyboxShader;
            pipelineSpec.DepthWrite = false;
			pipelineSpec.DepthTest = false;
            // pipelineSpec.Layout = {
			// 	{ ShaderDataType::Float3, "a_Position" },
			// };
			pipelineSpec.TargetFramebuffer = Framebuffer::Create(fbSpec);
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
			FramebufferSpecification fbSpec;
            fbSpec.ClearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
			fbSpec.Attachments = {
                ImageFormat::RGB16F,
                ImageFormat::RGB16F,
                ImageFormat::RGB16F,
                ImageFormat::RGB16F,
                ImageFormat::RGB16F,
                ImageFormat::RED32I,
                ImageFormat::Depth
            };
			// fbSpec.ExistingImages[0] = m_CompositePass->GetOutput(0);

			PipelineSpecification pipelineSpec;
			pipelineSpec.DebugName = "Geometry";
			// pipelineSpec.DepthOperator = DepthCompareOperator::Equal;
            pipelineSpec.DepthWrite = false;
            pipelineSpec.Shader = Renderer::GetShaderLibrary()->Get("Geometry");
			pipelineSpec.TargetFramebuffer = Framebuffer::Create(fbSpec);

			RenderPassSpecification renderPassSpec;
			renderPassSpec.DebugName = "Geometry";
			renderPassSpec.Pipeline = Pipeline::Create(pipelineSpec);
			m_GeometryPass = RenderPass::Create(renderPassSpec);
			m_GeometryPass->SetInput("CameraData", m_CameraUB);
        }

        // Solid
        {
            FramebufferSpecification fbSpec;
            fbSpec.ClearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
			fbSpec.Attachments = { ImageFormat::RGBA32F, ImageFormat::RGB16F, ImageFormat::RED32I, ImageFormat::Depth };

			PipelineSpecification pipelineSpec;
			pipelineSpec.DebugName = "Solid";
            pipelineSpec.DepthWrite = false;
            pipelineSpec.Shader = Renderer::GetShaderLibrary()->Get("Solid");
			pipelineSpec.TargetFramebuffer = Framebuffer::Create(fbSpec);
			m_SolidMaterial = Material::Create(pipelineSpec.Shader, pipelineSpec.DebugName);

			RenderPassSpecification renderPassSpec;
			renderPassSpec.DebugName = "Solid";
			renderPassSpec.Pipeline = Pipeline::Create(pipelineSpec);
			m_SolidPass = RenderPass::Create(renderPassSpec);
			m_SolidPass->SetInput("CameraData", m_CameraUB);
			m_SolidPass->SetInput("SceneData", m_SceneUB);
        }

        // ID
        {
            FramebufferSpecification fbSpec;
			fbSpec.Attachments = {
                ImageFormat::RGB8,
                ImageFormat::RED32I,
            };

			PipelineSpecification pipelineSpec;
			pipelineSpec.DebugName = "ID";
            pipelineSpec.DepthWrite = false;
            pipelineSpec.Shader = Renderer::GetShaderLibrary()->Get("ID");
			pipelineSpec.TargetFramebuffer = Framebuffer::Create(fbSpec);

			m_IDMaterial = Material::Create(pipelineSpec.Shader, pipelineSpec.DebugName);
            Ref<Texture2D> solidIdTex = m_SolidPass->GetOutput(2);
            Ref<Texture2D> solidDepthTex = m_SolidPass->GetOutput(1);
            Ref<Texture2D> PBRIdTex = m_GeometryPass->GetOutput(5);
            Ref<Texture2D> PBRDepthTex = m_GeometryPass->GetOutput(2);
            m_IDMaterial->Set("u_SolidIdTex", solidIdTex);
            m_IDMaterial->Set("u_SolidDepthTex", solidDepthTex);
            m_IDMaterial->Set("u_PBRIdTex", PBRIdTex);
            m_IDMaterial->Set("u_PBRDepthTex", PBRDepthTex);

			RenderPassSpecification renderPassSpec;
			renderPassSpec.DebugName = "ID";
			renderPassSpec.Pipeline = Pipeline::Create(pipelineSpec);
			m_IDPass = RenderPass::Create(renderPassSpec);
        }

        // Phong-Light
        {
            FramebufferSpecification fbSpec;
			fbSpec.Attachments = { ImageFormat::RGBA32F };
			// fbSpec.ExistingImages[0] = m_CompositePass->GetOutput(0);

			PipelineSpecification pipelineSpec;
			pipelineSpec.DebugName = "PhongLight";
            pipelineSpec.DepthWrite = false;
			pipelineSpec.TargetFramebuffer = Framebuffer::Create(fbSpec);
            pipelineSpec.Shader = Renderer::GetShaderLibrary()->Get("PhongLight");
			m_PhongLightMaterial = Material::Create(pipelineSpec.Shader, pipelineSpec.DebugName);
            Ref<Texture2D> positionTex = m_GeometryPass->GetOutput(0);
            Ref<Texture2D> normalTex = m_GeometryPass->GetOutput(1);
            Ref<Texture2D> baseColorTex = m_GeometryPass->GetOutput(3);
            Ref<Texture2D> materialPropTex = m_GeometryPass->GetOutput(4);
            m_PhongLightMaterial->Set("u_PositionTex", positionTex);
            m_PhongLightMaterial->Set("u_NormalTex", normalTex);
            m_PhongLightMaterial->Set("u_BaseColorTex", baseColorTex);
            m_PhongLightMaterial->Set("u_MaterialPropTex", materialPropTex);

			RenderPassSpecification renderPassSpec;
			renderPassSpec.DebugName = "PhongLight";
			renderPassSpec.Pipeline = Pipeline::Create(pipelineSpec);
			m_PhongLightPass = RenderPass::Create(renderPassSpec);
			m_PhongLightPass->SetInput("SceneData", m_SceneUB);
			m_PhongLightPass->SetInput("DirectionalLightsData", m_DirectionalLightsUB);
			m_PhongLightPass->SetInput("PointLightsData", m_PointLightsUB);
			m_PhongLightPass->SetInput("SpotLightsData", m_SpotLightsUB);
        }

        // PBR
        {
            FramebufferSpecification fbSpec;
            fbSpec.ClearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
			fbSpec.Attachments = { ImageFormat::RGBA16F };
			// fbSpec.ExistingImages[0] = m_CompositePass->GetOutput(0);

			PipelineSpecification pipelineSpec;
			pipelineSpec.DebugName = "PBR";
            pipelineSpec.DepthWrite = false;
			pipelineSpec.TargetFramebuffer = Framebuffer::Create(fbSpec);
            pipelineSpec.Shader = Renderer::GetShaderLibrary()->Get("PBR");
			m_PBRMaterial = Material::Create(pipelineSpec.Shader, pipelineSpec.DebugName);

			RenderPassSpecification renderPassSpec;
			renderPassSpec.DebugName = "PBR";
			renderPassSpec.Pipeline = Pipeline::Create(pipelineSpec);
			m_PBRPass = RenderPass::Create(renderPassSpec);
			m_PBRPass->SetInput("SceneData", m_SceneUB);
        	m_PBRPass->SetInput("DirectionalLightsData", m_DirectionalLightsUB);
			m_PBRPass->SetInput("PointLightsData", m_PointLightsUB);
			// m_PBRPass->SetInput("SpotLightData", m_SpotLightsUB);

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
            fbSpec.ClearColor = Renderer::GetConfig().ClearColor;
            fbSpec.Attachments = { ImageFormat::RGBA, ImageFormat::Depth };

            PipelineSpecification pipelineSpec;
            pipelineSpec.Layout = {
                { ShaderDataType::Float3, "a_Position" },
                { ShaderDataType::Float2, "a_TexCoord" }
            };
            pipelineSpec.BackfaceCulling = false;
            pipelineSpec.Shader = Renderer::GetShaderLibrary()->Get("SceneComposite");
            pipelineSpec.TargetFramebuffer = Framebuffer::Create(fbSpec);
            pipelineSpec.DebugName = "SceneComposite";
            pipelineSpec.DepthWrite = false;
            pipelineSpec.DepthTest = false;

			m_CompositeMaterial = Material::Create(pipelineSpec.Shader, pipelineSpec.DebugName);
            Ref<Texture2D> skyboxTex = m_SkyboxPass->GetOutput(0);
            Ref<Texture2D> solidTex = m_SolidPass->GetOutput(0);
            Ref<Texture2D> solidDepthTex = m_SolidPass->GetOutput(1);
            Ref<Texture2D> phongLightTex = m_PhongLightPass->GetOutput(0);
            Ref<Texture2D> PBRTex = m_PBRPass->GetOutput(0);
            Ref<Texture2D> PBRDepthTex = m_GeometryPass->GetOutput(2);
            m_CompositeMaterial->Set("u_SkyboxTex", skyboxTex);
            m_CompositeMaterial->Set("u_SolidTex", solidTex);
            m_CompositeMaterial->Set("u_SolidDepthTex", solidDepthTex);
            m_CompositeMaterial->Set("u_PBRTex", PBRTex);
            m_CompositeMaterial->Set("u_PBRDepthTex", PBRDepthTex);

            RenderPassSpecification renderPassSpec;
            renderPassSpec.DebugName = "SceneComposite";
            renderPassSpec.Pipeline = Pipeline::Create(pipelineSpec);
            m_CompositePass = RenderPass::Create(renderPassSpec);
        }
    }

    void SceneRenderer::Shutdown()
    {
    }

    void SceneRenderer::SetScene(Ref<Scene> scene)
    {
        m_Scene = scene;
    }

    void SceneRenderer::BeginScene(EditorCamera& camera)
    {
        m_CompositePass->GetTargetFramebuffer()->Resize(m_ViewportWidth, m_ViewportHeight);
        m_SkyboxPass->GetTargetFramebuffer()->Resize(m_ViewportWidth, m_ViewportHeight);
        m_GeometryPass->GetTargetFramebuffer()->Resize(m_ViewportWidth, m_ViewportHeight);
        m_IDPass->GetTargetFramebuffer()->Resize(m_ViewportWidth, m_ViewportHeight);
        m_SolidPass->GetTargetFramebuffer()->Resize(m_ViewportWidth, m_ViewportHeight);
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
        m_DirectionalLightsUB->SetData(&DirectionalLightsDataUB, sizeof(DirectionalLightsData));
        m_PointLightsUB->SetData(&PointLightsDataUB, sizeof(PointLightsData));
        m_SpotLightsUB->SetData(&SpotLightsDataUB, sizeof(SpotLightsData));
        DirectionalLightsDataUB.LightCount = 0;
        PointLightsDataUB.LightCount = 0;
        SpotLightsDataUB.LightCount = 0;
    }

    void SceneRenderer::EndScene()
    {
        Renderer::RenderStaticBatches();
        Flush();
    }

    void SceneRenderer::SetViewportSize(const float width, const float height)
    {
        bool inValid = width <= 0.0f || height <= 0.0f;
        if (inValid || (m_ViewportWidth == width && m_ViewportHeight == height))
            return;

        m_ViewportWidth = width;
        m_ViewportHeight = height;
    }

    bool SceneRenderer::SubmitDirectionalLight(DirectionalLightComponent *light)
    {
    	uint index = DirectionalLightsDataUB.LightCount;
    	if (index >= 1000)
    	{
    		CZ_CORE_WARN("DirectionalLightBuffer is full, cannot submit more point lights.");
    		return false;
    	}

    	DirectionalLightsDataUB.Lights[index].Direction = light->Direction;
    	DirectionalLightsDataUB.Lights[index].Intensity = light->Intensity;
    	DirectionalLightsDataUB.Lights[index].Color = light->Color;

    	DirectionalLightsDataUB.LightCount++;

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

    void SceneRenderer::SubmitMesh(Ref<DynamicMesh> mesh, uint32_t submeshIndex, Ref<Material> material, const glm::mat4& transform, uint64_t entityID)
    {
        MeshData meshData;
        meshData.Mesh = mesh;
        meshData.SubmeshIndex = submeshIndex;
        meshData.Material = material;
        meshData.Transform = transform;
        meshData.ID = entityID;
        m_MeshDatas.push_back(meshData);
    }

    void SceneRenderer::SkyboxPass()
    {
		RenderCommand::BeginRenderPass(m_CommandBuffer, m_SkyboxPass);

        m_SkyboxMaterial->Set("u_FragUniforms.TextureLod", m_SceneData.SkyboxLod);
		m_SkyboxMaterial->Set("u_FragUniforms.Intensity", m_SceneData.SceneEnvironmentIntensity);

		const Ref<TextureCube> radianceMap = m_SceneData.SceneEnvironment ? m_SceneData.SceneEnvironment->RadianceMap : Renderer::GetBlackTextureCube();
		m_SkyboxMaterial->Set("u_Texture", radianceMap);

		RenderCommand::SubmitFullscreenBox(m_CommandBuffer, m_SkyboxPass->GetPipeline(), m_SkyboxMaterial);
		RenderCommand::EndRenderPass(m_CommandBuffer, m_SkyboxPass);
    }

    void SceneRenderer::GeometryPass()
    {
		RenderCommand::BeginRenderPass(m_CommandBuffer, m_GeometryPass);
        for (auto meshData : m_MeshDatas)
        {
            if (!meshData.Material)
                continue;

            meshData.Material->Set("u_Material.ID", (int)meshData.ID);

            if (meshData.Mesh.As<DynamicMesh>())
                RenderCommand::SubmitMeshWithMaterial(m_CommandBuffer, m_GeometryPass->GetPipeline(), meshData.Mesh.As<DynamicMesh>(), meshData.SubmeshIndex, meshData.Material, meshData.Transform);
        }
		RenderCommand::EndRenderPass(m_CommandBuffer, m_GeometryPass);
    }

    void SceneRenderer::IDPass()
    {
		RenderCommand::BeginRenderPass(m_CommandBuffer, m_IDPass);
		RenderCommand::SubmitFullscreenQuad(m_CommandBuffer, m_IDPass->GetPipeline(), m_IDMaterial);
		RenderCommand::EndRenderPass(m_CommandBuffer, m_IDPass);
    }

    void SceneRenderer::SolidPass()
    {
		RenderCommand::BeginRenderPass(m_CommandBuffer, m_SolidPass);
        for (auto meshData : m_MeshDatas)
        {
            if (!meshData.Material)
            {
                auto material = Material::Copy(m_SolidMaterial);
                material->Set("u_Material.ID", (int)meshData.ID);

                if (meshData.Mesh.As<DynamicMesh>())
                    RenderCommand::SubmitMeshWithMaterial(m_CommandBuffer, m_SolidPass->GetPipeline(), meshData.Mesh.As<DynamicMesh>(), meshData.SubmeshIndex, material, meshData.Transform);
            }
        }
		RenderCommand::EndRenderPass(m_CommandBuffer, m_SolidPass);
    }

    void SceneRenderer::PhongLightPass()
    {
        RenderCommand::BeginRenderPass(m_CommandBuffer, m_PhongLightPass);
		RenderCommand::SubmitFullscreenQuad(m_CommandBuffer, m_PhongLightPass->GetPipeline(), m_PhongLightMaterial);
		RenderCommand::EndRenderPass(m_CommandBuffer, m_PhongLightPass);
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
        RenderCommand::BeginRenderPass(m_CommandBuffer, m_PBRPass);
       
        Ref<Texture2D> positionTex = m_GeometryPass->GetOutput(0);
        Ref<Texture2D> normalTex = m_GeometryPass->GetOutput(1);
        Ref<Texture2D> baseColorTex = m_GeometryPass->GetOutput(3);
        Ref<Texture2D> materialPropTex = m_GeometryPass->GetOutput(4);
        Ref<TextureCube> irradianceMap = Renderer::GetIrradianceTextureCube();
        Ref<TextureCube> prefilterMap = Renderer::GetPrefilteredTextureCube();
        Ref<Texture2D> brdfLUTTexture = Renderer::GetBRDFLutTexture();
        m_PBRMaterial->Set("u_PositionTex", positionTex);
        m_PBRMaterial->Set("u_NormalTex", normalTex);
        m_PBRMaterial->Set("u_BaseColorTex", baseColorTex);
        m_PBRMaterial->Set("u_MaterialPropTex", materialPropTex);
        m_PBRMaterial->Set("u_IrradianceMap", irradianceMap);
        m_PBRMaterial->Set("u_PrefilterMap", prefilterMap);
        m_PBRMaterial->Set("u_BRDFLutTex", brdfLUTTexture);

		RenderCommand::SubmitFullscreenQuad(m_CommandBuffer, m_PBRPass->GetPipeline(), m_PBRMaterial);
		RenderCommand::EndRenderPass(m_CommandBuffer, m_PBRPass);
    }

    void SceneRenderer::CompositePass()
    {
        m_CompositePass->GetTargetFramebuffer()->GetSpecification().ClearColor = Renderer::GetConfig().ClearColor;

		RenderCommand::BeginRenderPass(m_CommandBuffer, m_CompositePass);
		RenderCommand::SubmitFullscreenQuad(m_CommandBuffer, m_CompositePass->GetPipeline(), m_CompositeMaterial);
		RenderCommand::EndRenderPass(m_CommandBuffer, m_CompositePass);
    }

    void SceneRenderer::Flush()
    {
        m_CommandBuffer->Begin();

        SkyboxPass();
        GeometryPass();
        SolidPass();
        IDPass();
        // PhongLightPass();
        PBRPass();
        CompositePass();

        SceneRenderEvent event;
        m_EventBus.Dispatch(event);

        m_CommandBuffer->End();

        m_MeshDatas.clear();
    }

    void SceneRenderer::CopyImage(Ref<Texture2D> source, SharedBuffer &dest)
    {
        RenderCommand::CopyImage(m_CommandBuffer, source, dest);
    }
}

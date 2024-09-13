#include "SceneRenderer.h"

#include "Renderer.h"
#include "RenderCommand.h"

#include "Geometry/BoxGeometry.h"

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
#ifdef CZ_PIPELINE
        m_CommandBuffer = RenderCommandBuffer::Create();
#endif

        // Uniform buffers
        m_CameraUB = UniformBuffer::Create(sizeof(CameraData));
        m_SceneUB = UniformBuffer::Create(sizeof(SceneData));
        m_PointLightUB = UniformBuffer::Create(sizeof(PointLightsData));
        m_SpotLightUB = UniformBuffer::Create(sizeof(SpotLightsData));

#ifdef CZ_PIPELINE
        // Skybox
        {
			auto skyboxShader = Renderer::GetShaderLibrary()->Get("Skybox");

			FramebufferSpecification fbSpec;
			fbSpec.Attachments = { ImageFormat::RGBA32F };
			Ref<Framebuffer> skyboxFB = Framebuffer::Create(fbSpec);
            
			PipelineSpecification pipelineSpec;
			pipelineSpec.DebugName = "Skybox";
			pipelineSpec.Shader = skyboxShader;
            pipelineSpec.DepthWrite = false;
			pipelineSpec.DepthTest = false;
            pipelineSpec.Layout = {
				{ ShaderDataType::Float3, "a_Position" },
			};
			pipelineSpec.TargetFramebuffer = skyboxFB;
			m_SkyboxPipeline = Pipeline::Create(pipelineSpec);
			m_SkyboxMaterial = Material::Create(pipelineSpec.Shader, pipelineSpec.DebugName);

			RenderPassSpecification renderPassSpec;
			renderPassSpec.DebugName = "Skybox";
			renderPassSpec.Pipeline = m_SkyboxPipeline;
			m_SkyboxPass = RenderPass::Create(renderPassSpec);
			m_SkyboxPass->SetInput("Camera", m_CameraUB);
			// m_SkyboxPass->Bake();
        }
#endif
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

    void SceneRenderer::SkyboxPass()
    {
		Renderer::BeginRenderPass(m_CommandBuffer, m_SkyboxPass);

        m_SkyboxMaterial->Set("u_Uniforms.TextureLod", m_SceneData.SkyboxLod);
		m_SkyboxMaterial->Set("u_Uniforms.Intensity", m_SceneData.SceneEnvironmentIntensity);

		// const Ref<TextureCube> radianceMap = m_SceneData.SceneEnvironment ? m_SceneData.SceneEnvironment->RadianceMap : Renderer::GetBlackCubeTexture();
		// m_SkyboxMaterial->Set("u_Texture", radianceMap);

		Renderer::SubmitFullscreenBox(m_CommandBuffer, m_SkyboxPipeline, m_SkyboxMaterial);

		Renderer::EndRenderPass(m_CommandBuffer, m_SkyboxPass);
    }

    void SceneRenderer::Flush()
    {
        m_CommandBuffer->Begin();

        SkyboxPass();

        m_CommandBuffer->End();
        m_CommandBuffer->Submit();
    }

    void SceneRenderer::EndScene()
    {
        Renderer::RenderStaticBatches();
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

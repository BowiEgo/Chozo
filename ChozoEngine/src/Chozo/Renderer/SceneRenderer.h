#pragma once

#include "czpch.h"

#include "Mesh.h"
#include "UniformBuffer.h"
#include "RenderPass.h"
#include "EditorCamera.h"
#include "RenderCommandBuffer.h"

#include "Chozo/Scene/Scene.h"
#include "Chozo/Scene/Components.h"

#include "Chozo/Events/RenderEvent.h"

namespace Chozo
{

    struct DirLight
    {
        glm::vec3 Direction;
        float Intensity;
        glm::vec3 Color;
        float Padding;
    };
    struct PointLight
    {
        glm::vec3 Position;
        float Intensity;
        glm::vec3 Color;
        float Padding;
    };
    struct SpotLight
    {
        glm::vec3 Position = { 0.0f, 0.0f, 0.0f };
        float Intensity = 1.0f;
        glm::vec3 Direction = { 0.0f, 0.0f, -1.0f };
        float AngleAttenuation = 5.0f;
        glm::vec3 Color = { 1.0f, 1.0f, 1.0f };;
        float Angle = 10.0f;
    };

    class SceneRenderer : public RefCounted
    {
    public:
        SceneRenderer(Ref<Scene> scene);
        ~SceneRenderer();

		void Init();
        void Shutdown();
        
		Ref<Scene> GetScene() const { return m_Scene; }
		void SetScene(Ref<Scene> scene);
        void BeginScene(EditorCamera& camera); // TODO: Remove
        void EndScene(); // TODO: Remove

        void SetActive(bool active) { m_Active = active; }
        bool IsActive() const { return m_Active; }

        void AddEventListener(const EventType type, const EventCallback& cb, bool destroy = true) { m_EventBus.AddListener(type, cb, destroy); }

		void SetViewportSize(float width, float height);
    	void SetClearColor(glm::vec4 color);

        bool SubmitDirectionalLight(DirectionalLightComponent* light);
        bool SubmitPointLight(PointLightComponent* light, glm::vec3& position);
        bool SubmitSpotLight(SpotLightComponent* light, glm::vec3& position);

        void SubmitMesh(Ref<DynamicMesh> mesh, uint32_t submeshIndex, Ref<Material> material, const glm::mat4& transform, uint64_t entityID);

        Ref<RenderPass> GetSkyboxPass() { return m_SkyboxPass; }
        Ref<RenderPass> GetGeometryPass() { return m_GeometryPass; }
        Ref<RenderPass> GetIDPass() { return m_IDPass; }
        Ref<RenderPass> GetSolidPass() { return m_SolidPass; }
        Ref<RenderPass> GetPhongPass() { return m_PhongPass; }
        Ref<RenderPass> GetPBRPass() { return m_PBRPass; }
        Ref<RenderPass> GetCompositePass() { return m_CompositePass; }

        Ref<TextureCube> GetPBRIrradiance() { return m_PBRIrradiance; }

        void SkyboxPass();
        void GeometryPass();
        void IDPass(); // TODO: Only supported in editor mode.
        void SolidPass();
        void PhongPass();
        void PBRPrePass();
        void PBRPass();
        void CompositePass();

        void Flush();
        void CopyImage(Ref<Texture2D> source, SharedBuffer& dest);
    private:
		Ref<Scene> m_Scene;
		bool m_Active = false;

        EventBus m_EventBus;

		Ref<RenderCommandBuffer> m_CommandBuffer;

        struct MeshData
        {
            Ref<Mesh> Mesh;
            uint32_t SubmeshIndex;
            Ref<Material> Material;
            glm::mat4 Transform;
            uint64_t ID;
        };

        std::vector<MeshData> m_MeshDatas;

        struct SceneInfo
		{
			EditorCamera SceneCamera;

			// Resources
			Ref<Environment> SceneEnvironment;
			float SkyboxLod = 0.0f;
			float SceneEnvironmentIntensity;
		} m_SceneData;

        struct SceneData
		{
			glm::vec3 CameraPosition;
			float EnvironmentMapIntensity = 1.0f;
		} SceneDataUB;

        struct CameraData
        {
            glm::mat4 ProjectionMatrix;
            glm::mat4 ViewMatrix;
            glm::mat4 InverseViewProjectionMatrix;
        } CameraDataUB;

    	struct DirectionalLightsData
    	{
    		uint LightCount = 0;
    		float Padding[3];
    		DirLight Lights[1000];
    	} DirectionalLightsDataUB;

        struct PointLightsData
        {
            uint LightCount = 0;
            float Padding[3];
            PointLight Lights[1000];
        } PointLightsDataUB;

        struct SpotLightsData
        {
            uint LightCount = 0;
            float Padding[3];
            SpotLight Lights[1000];
        } SpotLightsDataUB;

        Ref<UniformBuffer> m_CameraUB;
        Ref<UniformBuffer> m_SceneUB;
        Ref<UniformBuffer> m_PointLightsUB;
        Ref<UniformBuffer> m_DirectionalLightsUB;
        Ref<UniformBuffer> m_SpotLightsUB;

		Ref<Material> m_SkyboxMaterial;
		Ref<RenderPass> m_SkyboxPass;

		Ref<RenderPass> m_GeometryPass;
        
		Ref<Material> m_IDMaterial;
		Ref<RenderPass> m_IDPass;

		Ref<Material> m_SolidMaterial;
		Ref<RenderPass> m_SolidPass;

		Ref<Material> m_PhongMaterial;
		Ref<RenderPass> m_PhongPass;

        Ref<TextureCube> m_PBRIrradiance;
        Ref<Material> m_PBRIrradianceMaterial, m_PBRMaterial;
		Ref<RenderPass> m_PBRIrradiancePass, m_PBRPass;

		Ref<Material> m_CompositeMaterial;
        Ref<RenderPass> m_CompositePass;

		float m_ViewportWidth = 0, m_ViewportHeight = 0;
    };
}

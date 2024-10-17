#pragma once

#include "czpch.h"

#include "Mesh.h"
#include "UniformBuffer.h"
#include "RenderPass.h"
#include "EditorCamera.h"
#include "RenderCommandBuffer.h"

#include "Chozo/Scene/Scene.h"
#include "Chozo/Scene/Components.h"

#include <glad/glad.h>

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
        ~SceneRenderer() = default;

		void Init();
        static void Shutdown();
        
		Ref<Scene> GetScene() const { return m_Scene; }
		void SetScene(Ref<Scene>& scene);
        void BeginScene(EditorCamera& camera); // TODO: Remove
        void EndScene(); // TODO: Remove

        inline void SetActive(bool active) { m_Active = active; }
        inline bool IsActive() const { return m_Active; }

        inline void AddEventListener(const EventType type, const EventCallback& cb, bool destroy = true) { m_EventBus.AddListener(type, cb, destroy); }

		void SetViewportSize(uint32_t width, uint32_t height);

        bool SubmitDirectionalLight(DirectionalLightComponent* light);
        bool SubmitPointLight(PointLightComponent* light, glm::vec3& position);
        bool SubmitSpotLight(SpotLightComponent* light, glm::vec3& position);

        void SubmitMesh(Ref<DynamicMesh> mesh, Ref<Material> material, const glm::mat4& transform);

        Ref<RenderPass> GetSkyboxPass() { return m_SkyboxPass; }
        Ref<RenderPass> GetGeometryPass() { return m_GeometryPass; }
        Ref<RenderPass> GetIDPass() { return m_IDPass; }
        Ref<RenderPass> GetSolidPass() { return m_SolidPass; }
        Ref<RenderPass> GetPhongLightPass() { return m_PhongLightPass; }
        Ref<RenderPass> GetCompositePass() { return m_CompositePass; }

        Ref<TextureCube> GetPBRIrradiance() { return m_PBRIrradiance; }

        void SkyboxPass();
        void GeometryPass();
        void IDPass(); // TODO: Only supported in editor mode.
        void SolidPass();
        void PhongLightPass();
        void PBRPrePass();
        void PBRPass();
        void CompositePass();

        void Flush();
        void CopyCompositeImage(SharedBuffer& dest);

        static Ref<SceneRenderer> Create(Ref<Scene>& scene);
        static Ref<SceneRenderer> Find(Scene* scene);
    private:
        SceneRenderer(Ref<Scene>& scene);

		Ref<Scene> m_Scene;
		bool m_Active = false;

        EventBus m_EventBus;

		Ref<RenderCommandBuffer> m_CommandBuffer;

        struct MeshData
        {
            Ref<Mesh> Mesh;
            Ref<Material> Material;
            glm::mat4 Transform;
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
			DirLight Lights;
			glm::vec3 CameraPosition;
			float EnvironmentMapIntensity = 1.0f;
		} SceneDataUB;

        struct CameraData
        {
            glm::mat4 ProjectionMatrix;
            glm::mat4 ViewMatrix;
            glm::mat4 InverseViewProjectionMatrix;
        } CameraDataUB;

        struct PointLightsData
        {
            uint LightCount;
            float Padding[3];
            PointLight Lights[1000];
        } PointLightsDataUB;

        struct SpotLightsData
        {
            uint LightCount;
            float Padding[3];
            SpotLight Lights[1000];
        } SpotLightsDataUB;

        Ref<UniformBuffer> m_CameraUB;
        Ref<UniformBuffer> m_SceneUB;
        Ref<UniformBuffer> m_PointLightUB;
        Ref<UniformBuffer> m_SpotLightUB;

		Ref<Material> m_SkyboxMaterial;
		Ref<RenderPass> m_SkyboxPass;

		Ref<RenderPass> m_GeometryPass;
        
		Ref<Material> m_IDMaterial;
		Ref<RenderPass> m_IDPass;

		Ref<Material> m_SolidMaterial;
		Ref<RenderPass> m_SolidPass;

		Ref<Material> m_PhongLightMaterial;
		Ref<RenderPass> m_PhongLightPass;

        Ref<TextureCube> m_PBRIrradiance;
        Ref<Material> m_PBRIrradianceMaterial, m_PBRMaterial;
		Ref<RenderPass> m_PBRIrradiancePass, m_PBRPass;

		Ref<Material> m_CompositeMaterial;
        Ref<RenderPass> m_CompositePass;

		uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;
    };
}

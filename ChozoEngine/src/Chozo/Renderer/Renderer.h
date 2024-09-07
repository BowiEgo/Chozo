#pragma once

#include "czpch.h"

#include "EditorCamera.h"
#include "RenderCommand.h"
#include "OrthographicCamera.h"
#include "Shader.h"
#include "Texture.h"
#include "Material.h"
#include "Mesh.h"
#include "UniformBuffer.h"
#include "RenderPass.h"

#include "Batch.h"

#include "Chozo/Scene/Components.h"

namespace Chozo {

    class Renderer
    {
    public:

        struct RendererConfig
        {
            // Tiering settings
            uint32_t EnvironmentMapResolution = 1024;
            uint32_t IrradianceMapComputeSamples = 512;
        };

        struct Statistics
        {
            uint32_t DrawCalls = 0;
            uint32_t VerticesCount = 0;
            uint32_t TriangleCount = 0;

            uint32_t GetTotalVerticesCount() { return VerticesCount; }
            uint32_t GetTotalTrianglesCount() { return TriangleCount; }
        };

        static Statistics GetStats();
        static void ResetStats();

        struct RenderCamera
        {
            glm::mat4 PrjectionMatrix;
            glm::mat4 ViewMatrix;

        };

        struct RendererData
        {
            uint32_t IndexCount = 0;
            BatchManager BatchManager;

    		Ref<ShaderLibrary> m_ShaderLibrary;
    		// Ref<MaterialLibrary> m_MaterialLibrary;
            int MaxTextureSlots = 0;
            uint32_t TextureSlotIndex = 1; // 0 = white texture
            std::vector<Ref<Texture2D>> TextureSlots;
            Ref<Texture2D> WhiteTexture;

            struct CameraData
            {
                glm::mat4 ProjectionMatrix;
                glm::mat4 ViewMatrix;
                glm::mat4 InverseViewProjectionMatrix;
            };
            CameraData CameraBuffer;
            Ref<UniformBuffer> CameraUniformBuffer;

            struct DirectionalLight
            {
                glm::vec3 Direction;
                float Padding;
                glm::vec3 Color;
                float Multiplier;
            };
            struct SceneData
            {
                DirectionalLight DirectionalLights;
                glm::vec3 CameraPosition;
                float EnvironmentMapIntensity;
                float padding2[3];
            };
            SceneData SceneBuffer;
            Ref<UniformBuffer> SceneUniformBuffer;

            Renderer::Statistics Stats;

            uint32_t GetMaxTriangles() { return GetMaxCount<Index>(); }

            Ref<DynamicMesh> SkyBoxMesh;

		    Ref<RenderPass> m_PreethamSkyRenderPass;
        };

        static void Init();
        static void Shutdown();
        
        static void BeginScene(const glm::mat4& projection, const glm::mat4& transform);
        static void BeginScene(EditorCamera& camera); // TODO: Remove
        static void EndScene();

        static void RenderStaticBatches();
        static bool SubmitStaticMesh(StaticMesh* mesh);
        static bool RemoveStaticMesh(StaticMesh* mesh);
        static void DrawMesh(const glm::mat4 transform, DynamicMesh* mesh, Material* material, uint32_t entityID = -1);

        static bool SubmitDirectionalLight(DirectionalLightComponent* light);

        inline static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }

        static RendererData GetRendererData();
        static void UpdateMaxTriagles(uint32_t count);

		static void RenderPreethamSky(float turbidity, float azimuth, float inclination);
		static void CreatePreethamSky(float turbidity, float azimuth, float inclination);

        static RendererConfig& GetConfig();
		static void SetConfig(const RendererConfig& config);

        static void DrawSkyLight(Ref<Environment>& environment, float& environmentIntensity, float& skyboxLod, EditorCamera& camera);
    };
}
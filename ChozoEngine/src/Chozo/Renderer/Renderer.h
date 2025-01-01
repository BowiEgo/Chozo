#pragma once

#include "czpch.h"
#include <future>

#include "EditorCamera.h"
#include "RenderCommand.h"
#include "OrthographicCamera.h"
#include "Shader.h"
#include "Texture.h"
#include "Material.h"
#include "Mesh.h"
#include "UniformBuffer.h"
#include "RenderPass.h"
#include "RenderCommandBuffer.h"

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

            glm::vec4 ClearColor = { 0.105f, 0.110f, 0.110f, 1.0f };
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

            uint32_t MaxTextureSlots = 0;
            uint32_t TextureSlotIndex = 1; // 0 = white texture
            std::vector<Ref<Texture2D>> TextureSlots;
            Ref<Texture2D> WhiteTexture;
            Ref<Texture2D> CheckerboardTexture;
            Ref<Texture2D> BrdfLUT;
            Ref<TextureCube> BlackTextureCube;

            Ref<Geometry> QuadMesh;
            Ref<Geometry> BoxMesh;

            Renderer::Statistics Stats;

            uint32_t GetMaxTriangles() { return GetMaxCount<Index>(); }

            Ref<TextureCube> PreethamSkyTextureCube;
            Ref<TextureCube> StaticSkyTextureCube;
            Ref<TextureCube> IrradianceTextureCube;
            Ref<TextureCube> PrefilteredTextureCube;
            Ref<Material> m_IrradianceMaterial;
            Ref<Material> m_PrefilteredMaterial;
            Ref<Pipeline> m_PreethamSkyPipeline;
            Ref<Pipeline> m_CubemapSamplerPipeline;
            Ref<Pipeline> m_IrradiancePipeline;
            Ref<Pipeline> m_PrefilteredPipeline;
            Ref<Pipeline> m_BrdfLUTPipeline;
        };

        static void Init();
        static void Shutdown();
        
        static void DrawMesh(const glm::mat4 &transform, const DynamicMesh* mesh, Material* material, uint32_t entityID = -1); // TODO: Remove

        static Ref<ShaderLibrary> GetShaderLibrary() { return GetRendererData().m_ShaderLibrary; }
        static RendererData GetRendererData();
        static Ref<Texture2D> GetBrdfLUT();
        static Ref<Texture2D> GetCheckerboardTexture();
        static Ref<TextureCube> GetBlackTextureCube();
        static Ref<TextureCube> GetStaticSkyTextureCube();
        static Ref<TextureCube> GetIrradianceTextureCube();
        static Ref<TextureCube> GetPrefilteredTextureCube();
        static Ref<TextureCube> GetPreethamSkyTextureCube();
        static void UpdateMaxTriangles(uint32_t count);

        static RendererConfig& GetConfig();
		static void SetConfig(const RendererConfig& config);

        static uint32_t GetMaxTextureSlotCount();

        static Ref<TextureCube> CreateCubemap(const std::string& filePath);
		static void CreateStaticSky( const Ref<Texture2D>& texture);
		static void CreatePreethamSky(const float turbidity, const float azimuth, const float inclination);
		static void UpdatePreethamSky(const float turbidity, const float azimuth, const float inclination);

		static void Begin();
		static void End();
        static void Submit(std::function<void()>&& func);
        static void DebouncedSubmit(std::function<void()>&& func, uint32_t delay = 100);
    };
}
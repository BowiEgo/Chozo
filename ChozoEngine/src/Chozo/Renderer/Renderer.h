#pragma once

#include "czpch.h"

#include "RenderCommand.h"
#include "Shader/Shader.h"
#include "Texture.h"
#include "Material.h"
#include "Mesh.h"
#include "UniformBuffer.h"

#include "Batch.h"

#include "Chozo/Scene/Components.h"
#include "Chozo/Renderer/Shader/MaterialParams.h"

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
            uint32_t IndexCount = 0;
            uint32_t TriangleCount = 0;

            uint32_t GetTotalVerticesCount() const { return VerticesCount; }
            uint32_t GetTotalTrianglesCount() const { return TriangleCount; }
        };

        static Statistics GetStats();
        static void ResetStats();

        struct RenderCamera
        {
            glm::mat4 ProjectionMatrix;
            glm::mat4 ViewMatrix;
        };

        struct MaterialParamsData
        {
            uint32_t MaterialCount = 0;
            alignas(16) MaterialParams Materials[1000];
        };

        struct RendererData
        {
            uint32_t MaxTextureSlots = 0;
            uint32_t TextureSlotIndex = 1; // 0 = white texture
            std::vector<Ref<Texture2D>> TextureSlots;
            Ref<Texture2D> WhiteTexture;
            Ref<Texture2D> CheckerboardTexture;
            Ref<Texture2D> BrdfLUT;
            Ref<TextureCube> BlackTextureCube;

            Ref<Geometry> QuadMesh;
            Ref<Geometry> BoxMesh;

            Ref<ShaderLibrary> m_ShaderLibrary;

            Ref<UniformBuffer> m_MaterialParamsUB;

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

            Statistics Stats;
        };

        static void Init();
        static void Shutdown();
        
        static RendererData GetRendererData();
        static Ref<ShaderLibrary> GetShaderLibrary() { return GetRendererData().m_ShaderLibrary; }
        static Ref<UniformBuffer> GetMaterialParams() { return GetRendererData().m_MaterialParamsUB; }
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
		static void CreatePreethamSky(float turbidity, float azimuth, float inclination);
		static void UpdatePreethamSky(float turbidity, float azimuth, float inclination);

		static void Begin();
		static void End();
        static void Submit(std::function<void()>&& func);
        static bool SubmitMaterial(Ref<Material> material);
        static void UpdateMaterialParams(const std::string& name, const UniformValue& value, MaterialParams& matParams);
        static void DebouncedSubmit(std::function<void()>&& func, uint32_t delay = 100);
    };
}
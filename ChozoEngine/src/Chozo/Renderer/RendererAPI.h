#pragma once

#include <glm/glm.hpp>

#include "VertexArray.h"
#include "Shader.h"
#include "Material.h"
#include "Environment.h"
#include "EditorCamera.h"
#include "RenderCommandBuffer.h"
#include "RenderPass.h"
#include "Pipeline.h"

namespace Chozo {
    
    class RendererAPI
    {
    public:
        enum class API
        {
            None = 0, OpenGL = 1
        };
    public:
        virtual void Init() = 0;
        
        virtual uint32_t GetMaxTextureSlots() = 0;

        virtual void SetClearColor(const glm::vec4& color) = 0;
        virtual void Clear() = 0;

        virtual void DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount) = 0;
        virtual void DrawLines(const Ref<VertexArray>& vertexArray, uint32_t vertexCount) = 0;

        virtual void RenderCubemap(Ref<Pipeline> pipeline, Ref<TextureCube> cubemap, const std::string filePath) = 0;
        virtual void RenderCubemap(Ref<Pipeline> pipeline, Ref<TextureCube> cubemap, Ref<Material> material = nullptr) = 0;
        virtual void RenderPrefilteredCubemap(Ref<Pipeline> pipeline, Ref<TextureCube> cubemap, Ref<Material> material = nullptr) = 0;
        virtual void CreatePreethamSky(Ref<Pipeline> pipeline, const float turbidity, const float azimuth, const float inclination) = 0;
        virtual void DrawPreethamSky(Ref<Pipeline> pipeline, const float turbidity, const float azimuth, const float inclination) = 0;
        virtual void DrawEnvMap(const Ref<Shader>& shader, const Ref<TextureCube>& textureCube, const Ref<VertexArray>& VAO) = 0;
        virtual void DrawSkyLight(const Ref<Environment>& environment, const float& environmentIntensity, const float& skyboxLod, const EditorCamera& camera) = 0;

        virtual void BeginRenderPass(Ref<RenderCommandBuffer> commandBuffer, Ref<RenderPass> renderPass) = 0;
        virtual void EndRenderPass(Ref<RenderCommandBuffer> commandBuffer, Ref<RenderPass> renderPass) = 0;

        virtual void SubmitCubeMap(Ref<RenderCommandBuffer> commandBuffer, Ref<Pipeline> pipeline, Ref<TextureCube> cubemap, Ref<Material> material = nullptr) = 0;
        virtual void RenderFullscreenQuad(Ref<Pipeline> pipeline, Ref<Material> material = nullptr) = 0;
        virtual void SubmitFullscreenQuad(Ref<RenderCommandBuffer> commandBuffer, Ref<Pipeline> pipeline, Ref<Material> material = nullptr) = 0;
        virtual void SubmitFullscreenBox(Ref<RenderCommandBuffer> commandBuffer, Ref<Pipeline> pipeline, Ref<Material> material = nullptr) = 0;
        virtual void SubmitMeshWithMaterial(Ref<RenderCommandBuffer> commandBuffer, Ref<Pipeline> pipeline, Ref<DynamicMesh> mesh, Ref<Material> material) = 0;

        inline static API GetAPI() { return s_API; }
    private:
        static API s_API;
    };
}
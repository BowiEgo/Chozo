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
    
    class RenderAPI
    {
    public:
        enum class Type
        {
            None = 0, OpenGL = 1
        };
    public:
        virtual ~RenderAPI() = default;
        virtual void Init() = 0;
        
        virtual uint32_t GetMaxTextureSlotCount() = 0;

        virtual void SetClearColor(const glm::vec4& color) = 0;
        virtual void Clear() = 0;

        virtual void DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount, uint32_t indexOffset, uint32_t vertexOffset) = 0;
        virtual void DrawLines(const Ref<VertexArray>& vertexArray, uint32_t vertexCount) = 0;

        virtual void RenderCubemap(Ref<Pipeline> pipeline, Ref<TextureCube> cubemap, const Ref<Texture2D> texture) = 0;
        virtual void RenderCubemap(Ref<Pipeline> pipeline, Ref<TextureCube> cubemap, Ref<Material> material = nullptr) = 0;
        virtual void RenderPrefilteredCubemap(Ref<Pipeline> pipeline, Ref<TextureCube> cubemap, Ref<Material> material = nullptr) = 0;
        virtual void CreatePreethamSky(Ref<Pipeline> pipeline, const float turbidity, const float azimuth, const float inclination) = 0;
        virtual void DrawPreethamSky(Ref<Pipeline> pipeline, const float turbidity, const float azimuth, const float inclination) = 0;
        virtual void DrawEnvMap(const Ref<Shader>& shader, const Ref<TextureCube>& textureCube, const Ref<VertexArray>& VAO) = 0;
        virtual void DrawSkyBox(const Ref<Environment>& environment, const float& environmentIntensity, const float& skyboxLod, const EditorCamera& camera) = 0;

        virtual void BeginRenderPass(Ref<RenderCommandBuffer> commandBuffer, Ref<RenderPass> renderPass) = 0;
        virtual void EndRenderPass(Ref<RenderCommandBuffer> commandBuffer, Ref<RenderPass> renderPass) = 0;

        virtual void SubmitCubeMap(Ref<RenderCommandBuffer> commandBuffer, Ref<Pipeline> pipeline, Ref<TextureCube> cubemap, Ref<Material> material = nullptr) = 0;
        virtual void RenderFullscreenQuad(Ref<Pipeline> pipeline, Ref<Material> material = nullptr) = 0;
        virtual void SubmitFullscreenQuad(Ref<RenderCommandBuffer> commandBuffer, Ref<Pipeline> pipeline, Ref<Material> material = nullptr) = 0;
        virtual void SubmitFullscreenBox(Ref<RenderCommandBuffer> commandBuffer, Ref<Pipeline> pipeline, Ref<Material> material = nullptr) = 0;
        virtual void SubmitMeshWithMaterial(Ref<RenderCommandBuffer> commandBuffer, Ref<Pipeline> pipeline, Ref<DynamicMesh> mesh, uint32_t submeshIndex, Ref<Material> material, glm::mat4 transform, int id) = 0;

        virtual void CopyImage(Ref<RenderCommandBuffer> commandBuffer, Ref<Texture2D> source, SharedBuffer& dest) = 0;
    };

        class RenderCommand
    {
    public:
        static void Init() { s_API->Init(); }
        static void SwitchAPI(RenderAPI::Type api);
        inline static RenderAPI::Type GetType() { return s_Type; }

        inline static uint32_t GetMaxTextureSlotCount()  { return s_API->GetMaxTextureSlotCount(); }

        inline static void SetClearColor(const glm::vec4& color) { s_API->SetClearColor(color); }
        inline static void Clear() { s_API->Clear(); }

        inline static void DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount, uint32_t indexOffset = 0, uint32_t vertexOffset = 0) { s_API->DrawIndexed(vertexArray, indexCount, indexOffset, vertexOffset); }
        inline static void DrawLines(const Ref<VertexArray>& vertexArray, uint32_t vertexCount) { s_API->DrawLines(vertexArray, vertexCount); }

        inline static void RenderCubemap(Ref<Pipeline> pipeline, Ref<TextureCube> cubemap, const Ref<Texture2D> texture) { s_API->RenderCubemap(pipeline, cubemap, texture); }
        inline static void RenderCubemap(Ref<Pipeline> pipeline, Ref<TextureCube> cubemap, Ref<Material> material = nullptr) { s_API->RenderCubemap(pipeline, cubemap, material); }
        inline static void RenderPrefilteredCubemap(Ref<Pipeline> pipeline, Ref<TextureCube> cubemap, Ref<Material> material = nullptr) { s_API->RenderPrefilteredCubemap(pipeline, cubemap, material); }
        inline static void CreatePreethamSky(Ref<Pipeline> pipeline, const float turbidity, const float azimuth, const float inclination) { s_API->CreatePreethamSky(pipeline, turbidity, azimuth, inclination); }
        inline static void DrawPreethamSky(Ref<Pipeline> pipeline, const float turbidity, const float azimuth, const float inclination) { s_API->DrawPreethamSky(pipeline, turbidity, azimuth, inclination); }
        inline static void DrawEnvMap(const Ref<Shader>& shader, const Ref<TextureCube>& textureCube, const Ref<VertexArray>& VAO) { s_API->DrawEnvMap(shader, textureCube, VAO); }
        inline static void DrawSkyLight(const Ref<Environment>& environment, const float& environmentIntensity, const float& skyboxLod, const EditorCamera& camera)  { s_API->DrawSkyBox(environment, environmentIntensity, skyboxLod, camera); }

        inline static void BeginRenderPass(Ref<RenderCommandBuffer> commandBuffer, Ref<RenderPass> renderPass) { s_API->BeginRenderPass(commandBuffer, renderPass); }
        inline static void EndRenderPass(Ref<RenderCommandBuffer> commandBuffer, Ref<RenderPass> renderPass) { s_API->EndRenderPass(commandBuffer, renderPass); }

        inline static void SubmitCubeMap(Ref<RenderCommandBuffer> commandBuffer, Ref<Pipeline> pipeline, Ref<TextureCube> cubemap, Ref<Material> material = nullptr) { s_API->SubmitCubeMap(commandBuffer, pipeline, cubemap, material); }
        inline static void RenderFullscreenQuad(Ref<Pipeline> pipeline, Ref<Material> material = nullptr) { s_API->RenderFullscreenQuad(pipeline, material); }
        inline static void SubmitFullscreenQuad(Ref<RenderCommandBuffer> commandBuffer, Ref<Pipeline> pipeline, Ref<Material> material = nullptr) { s_API->SubmitFullscreenQuad(commandBuffer, pipeline, material); }
        inline static void SubmitFullscreenBox(Ref<RenderCommandBuffer> commandBuffer, Ref<Pipeline> pipeline, Ref<Material> material = nullptr) { s_API->SubmitFullscreenBox(commandBuffer, pipeline, material); }
        inline static void SubmitMeshWithMaterial(Ref<RenderCommandBuffer> commandBuffer, Ref<Pipeline> pipeline, Ref<DynamicMesh> mesh, uint32_t submeshIndex, Ref<Material> material, glm::mat4 transform, int id) { s_API->SubmitMeshWithMaterial(commandBuffer, pipeline, mesh, submeshIndex, material, transform, id); }

        inline static void CopyImage(Ref<RenderCommandBuffer> commandBuffer, Ref<Texture2D> source, SharedBuffer& dest){ s_API->CopyImage(commandBuffer, source, dest); }
    private:
        static RenderAPI::Type s_Type;
        static RenderAPI* s_API;
    };
}
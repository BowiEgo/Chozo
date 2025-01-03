#pragma once

#include "Chozo/Renderer/RenderCommand.h"
#include "Chozo/Renderer/Texture.h"

namespace Chozo {
    
    class OpenGLRenderAPI : public RenderAPI
    {
    public:
        virtual void Init() override;
        
        virtual uint32_t GetMaxTextureSlots() override;

        virtual void SetClearColor(const glm::vec4& color) override;
        virtual void Clear() override;

        virtual void DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount = 0, uint32_t indexOffset = 0, uint32_t vertexOffset = 0) override;
        virtual void DrawLines(const Ref<VertexArray>& vertexArray, uint32_t vertexCount) override;

        virtual void RenderCubemap(Ref<Pipeline> pipeline, Ref<TextureCube> cubemap, const Ref<Texture2D> texture) override;
        virtual void RenderCubemap(Ref<Pipeline> pipeline, Ref<TextureCube> cubemap, Ref<Material> material = nullptr) override;
        virtual void RenderPrefilteredCubemap(Ref<Pipeline> pipeline, Ref<TextureCube> cubemap, Ref<Material> material = nullptr) override;
        virtual void DrawPreethamSky(Ref<Pipeline> pipeline, const float turbidity, const float azimuth, const float inclination) override;
        virtual void CreatePreethamSky(Ref<Pipeline> pipeline, const float turbidity, const float azimuth, const float inclination) override;
        virtual void DrawEnvMap(const Ref<Shader>& shader, const Ref<TextureCube>& textureCube, const Ref<VertexArray>& VAO) override;
        virtual void DrawSkyBox(const Ref<Environment>& environment, const float& environmentIntensity, const float& skyboxLod, const EditorCamera& camera) override;

        virtual void BeginRenderPass(Ref<RenderCommandBuffer> commandBuffer, Ref<RenderPass> renderPass) override;
        virtual void EndRenderPass(Ref<RenderCommandBuffer> commandBuffer, Ref<RenderPass> renderPass) override;

        virtual void SubmitCubeMap(Ref<RenderCommandBuffer> commandBuffer, Ref<Pipeline> pipeline, Ref<TextureCube> cubemap, Ref<Material> material = nullptr) override;
        virtual void RenderFullscreenQuad(Ref<Pipeline> pipeline, Ref<Material> material = nullptr) override;
        virtual void SubmitFullscreenQuad(Ref<RenderCommandBuffer> commandBuffer, Ref<Pipeline> pipeline, Ref<Material> material = nullptr) override;
        virtual void SubmitFullscreenBox(Ref<RenderCommandBuffer> commandBuffer, Ref<Pipeline> pipeline, Ref<Material> material = nullptr) override;
        virtual void SubmitMeshWithMaterial(Ref<RenderCommandBuffer> commandBuffer, Ref<Pipeline> pipeline, Ref<DynamicMesh> mesh, uint32_t submeshIndex, Ref<Material> material, glm::mat4 transform, int id) override;

        virtual void CopyImage(Ref<RenderCommandBuffer> commandBuffer, Ref<Texture2D> source, SharedBuffer& dest) override;
    private:
        void PrepareGLContext(Ref<Pipeline> pipeline);
        void ResetGLContext();
    };
}
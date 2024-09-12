#pragma once

#include "Chozo/Renderer/RendererAPI.h"
#include "Chozo/Renderer/Texture.h"

namespace Chozo {
    
    class OpenGLRendererAPI : public RendererAPI
    {
    public:
        virtual void Init() override;
        
        virtual int GetMaxTextureSlots() override;

        virtual void SetClearColor(const glm::vec4& color) override;
        virtual void Clear() override;

        virtual void DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount) override;
        virtual void DrawLines(const Ref<VertexArray>& vertexArray, uint32_t vertexCount) override;

        virtual void DrawPreethamSky(const float turbidity, const float azimuth, const float inclination) override;
        virtual void DrawEnvMap(const Ref<Shader>& shader, const Ref<TextureCube>& textureCube, const Ref<VertexArray>& VAO) override;
        virtual void DrawSkyLight(const Ref<Environment>& environment, const float& environmentIntensity, const float& skyboxLod, const EditorCamera& camera) override;

        virtual void BeginRenderPass(Ref<RenderCommandBuffer> commandBuffer, Ref<RenderPass> renderPass) override;
        virtual void EndRenderPass(Ref<RenderCommandBuffer> commandBuffer, Ref<RenderPass> renderPass) override;

        virtual void SubmitFullscreenBox(Ref<RenderCommandBuffer> commandBuffer, Ref<Pipeline> pipeline, Ref<Material> material) override;
    };
}
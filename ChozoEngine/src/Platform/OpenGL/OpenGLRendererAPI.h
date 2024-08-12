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

        virtual Ref<TextureCube> CreatePreethamSky(float turbidity, float azimuth, float inclination) override;
    };
}
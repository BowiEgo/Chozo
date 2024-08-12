#pragma once

#include <glm/glm.hpp>

#include "VertexArray.h"
#include "Shader.h"
#include "Texture.h"

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
        
        virtual int GetMaxTextureSlots() = 0;

        virtual void SetClearColor(const glm::vec4& color) = 0;
        virtual void Clear() = 0;

        virtual void DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount) = 0;
        virtual void DrawLines(const Ref<VertexArray>& vertexArray, uint32_t vertexCount) = 0;

        virtual Ref<TextureCube> CreatePreethamSky(float turbidity, float azimuth, float inclination) = 0;
    
        inline static API GetAPI() { return s_API; }
    private:
        static API s_API;
    };
}
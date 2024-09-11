#pragma once

#include <glm/glm.hpp>

#include "VertexArray.h"
#include "Shader.h"
#include "Texture.h"
#include "Environment.h"
#include "EditorCamera.h"

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

        virtual void DrawPreethamSky(const float turbidity, const float azimuth, const float inclination) = 0;
        virtual void DrawEnvMap(const Ref<Shader>& shader, const Ref<TextureCube>& textureCube, const Ref<VertexArray>& VAO) = 0;
        virtual void DrawSkyLight(const Ref<Environment>& environment, const float& environmentIntensity, const float& skyboxLod, const EditorCamera& camera) = 0;

        inline static API GetAPI() { return s_API; }
    private:
        static API s_API;
    };
}
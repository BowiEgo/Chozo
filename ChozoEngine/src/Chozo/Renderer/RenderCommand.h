#pragma once

#include "RendererAPI.h"

namespace Chozo {

    class RenderCommand
    {
    public:
        inline static void Init()
        {
            s_RendererAPI->Init();
        }

        inline static int GetMaxTextureSlots()
        {
            return s_RendererAPI->GetMaxTextureSlots();
        }

        inline static void SetClearColor(const glm::vec4& color)
        {
            s_RendererAPI->SetClearColor(color);
        }

        inline static void Clear()
        {
            s_RendererAPI->Clear();
        }

        inline static void DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount = 0)
        {
            s_RendererAPI->DrawIndexed(vertexArray, indexCount);
        }

        inline static void DrawLines(const Ref<VertexArray>& vertexArray, uint32_t vertexCount = 0)
        {
            s_RendererAPI->DrawLines(vertexArray, vertexCount);
        }

        inline static void DrawEnvMap(const Ref<Shader>& shader, const Ref<TextureCube>& textureCube, const Ref<VertexArray>& VAO)
        {
            s_RendererAPI->DrawEnvMap(shader, textureCube, VAO);
        }

        inline static void DrawSkyLight(Ref<Environment>& environment, float& environmentIntensity, float& skyboxLod, const EditorCamera& camera)
        {
            s_RendererAPI->DrawSkyLight(environment, environmentIntensity, skyboxLod, camera);
        }
    private:
        static RendererAPI* s_RendererAPI;
    };
}
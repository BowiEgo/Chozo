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
    private:
        static RendererAPI* s_RendererAPI;
    };
}
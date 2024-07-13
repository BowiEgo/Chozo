#pragma once

#include "czpch.h"

#include "RenderCommand.h"
#include "OrthographicCamera.h"
#include "Shader.h"
#include "Texture.h"

namespace Chozo {

    class Renderer2D
    {
    public:
        static void Init();
        static void Shutdown();
        
        static void BeginScene(OrthographicCamera& camera);
        static void EndScene();

        static void BeginBatch();
        static void EndBatch();
        static void Flush();
        // Primitives
        static void DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color);
        // static void DrawQuad(const glm::vec2& position, const glm::vec2& size, const Ref<Texture2D>& texture, float tilingFactor = 1.0f, const glm::vec4& tintColor = glm::vec4(1.0f));

        static void Submit(const Ref<Shader>& shader);
    
        inline static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }

        // Stats
        struct Statistics
        {
            uint32_t DrawCalls = 0;
            uint32_t QuadCount = 0;

            uint32_t GetTotalVertexCount() { return QuadCount * 4; }
            uint32_t GetTotalIndexCount() { return QuadCount * 6; }
        };

        static Statistics GetStats();
        static void ResetStats();
    private:
        static void FlushAndReset();
    private:
        struct SceneData
        {
            glm::mat4 ViewProjectionMatrix;
            Ref<Shader> Shader;
        };

        static SceneData* m_SceneData;
    };
}
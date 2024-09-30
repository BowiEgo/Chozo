#pragma once

#include "czpch.h"

#include "EditorCamera.h"
#include "RenderCommand.h"
#include "OrthographicCamera.h"
#include "Shader.h"
#include "Texture.h"

#include "Chozo/Scene/Components.h"

namespace Chozo {

    class Renderer2D
    {
    public:
        struct RenderCamera
        {
            glm::mat4 PrjectionMatrix;
            glm::mat4 ViewMatrix;
        };

        static void Init();
        static void Shutdown();
        
        static void BeginScene(const glm::mat4& projection, const glm::mat4& transform);
        static void BeginScene(EditorCamera& camera); // TODO: Remove
        static void EndScene();

        static void BeginBatch();
        static void NextBatch();
        static void Flush();
        // Primitives
        static void DrawFullScreenQuad();
        static void DrawQuad(const glm::mat4& transform, const glm::vec4& color, uint32_t entityID = -1);
        static void DrawQuad(const glm::mat4& transform, Ref<Texture2D> texture, float tilingFactor, const glm::vec4& color, uint32_t entityID = -1);
        static void DrawSprite(const glm::mat4& transform, SpriteRendererComponent& src, uint32_t entityID = -1);
        static void DrawCircle(const glm::mat4& transform, const glm::vec4& color, float thickness = 1.0f, float fade = 0.001f, uint32_t entityID = -1);
        static void DrawLine(const glm::vec3& p0, const glm::vec3& p1, const glm::vec4& color, uint32_t entityID = -1);
        static void DrawRect(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color, uint32_t entityID = -1);
        static void DrawRect(const glm::mat4& transform, const glm::vec4& color, uint32_t entityID = -1);

        static float GetLineWidth();
        static void SetLineWidth(float width);

        static void Submit(const Ref<Shader>& shader);
    
        // Stats
        struct Statistics
        {
            uint32_t DrawCalls = 0;
            uint32_t QuadCount = 0, CircleCount = 0, LineCount;

            uint32_t GetTotalVertexCount() { return QuadCount * 4; }
            uint32_t GetTotalIndexCount() { return QuadCount * 6; }
        };

        static Statistics GetStats();
        static void ResetStats();
    private:
        static void FlushAndReset();
        static void FlushQuadBuffer();
    private:
        struct SceneData
        {
            glm::mat4 ViewProjectionMatrix;
            Ref<Shader> Shader;
        };

        static SceneData* m_SceneData;
    };
}
#include "Renderer2D.h"

#include "Platform/OpenGL/OpenGLShader.h"
#include "UniformBuffer.h"

#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>

namespace Chozo {
    struct QuadVertex
    {
        glm::vec3 Position;
        glm::vec4 Color;
        glm::vec2 TexCoord;
        // float TexIndex;
        // float TilingFactor;
        int EntityID;
    };

    struct Renderer2DData
    {
        static const uint32_t MaxQuads = 20000;
        static const uint32_t MaxVertexs = MaxQuads * 4;
        static const uint32_t MaxIndices = MaxQuads * 6;
        static const uint32_t MaxTextureSlots = 32;

        Ref<VertexArray> QuadVAO;
        Ref<VertexBuffer> QuadVBO;
        Ref<Shader> TextureShader;
        Ref<Texture2D> WhiteTexture;

        uint32_t QuadIndexCount = 0;
        QuadVertex* QuadVertexBufferBase = nullptr;
        QuadVertex* QuadVertexBufferPtr = nullptr;

        std::array<Ref<Texture2D>, MaxTextureSlots> TextureSlots;
        uint32_t TextureSlotIndex = 1;

        glm::vec4 QuadVertexPositions[4] = {
            { -0.5f, -0.5f, 0.0f, 1.0f },
            { -0.5f,  0.5f, 0.0f, 1.0f },
            {  0.5f,  0.5f, 0.0f, 1.0f },
            {  0.5f, -0.5f, 0.0f, 1.0f }
        };

        glm::vec2 QuadTexCoords[4] = {
            { 0.0f, 0.0f },
            { 1.0f, 0.0f },
            { 1.0f, 1.0f },
            { 0.0f, 1.0f },
        };

        struct CameraData
        {
            glm::mat4 ProjectionMatrix;
            glm::mat4 ViewMatrix;
        };
        CameraData CameraBuffer;
        Ref<UniformBuffer> CameraUniformBuffer;

        Renderer2D::Statistics Stats;
    };

    static Renderer2DData s_Data;

    Renderer2D::SceneData* Renderer2D::m_SceneData = new Renderer2D::SceneData;

    void Renderer2D::Init()
    {
        RenderCommand::Init();

        s_Data.QuadVertexBufferBase = new QuadVertex[s_Data.MaxVertexs];

        s_Data.QuadVAO = VertexArray::Create();
        s_Data.QuadVBO = VertexBuffer::Create(s_Data.MaxVertexs * sizeof(QuadVertex));

        s_Data.QuadVAO->Bind();
        s_Data.QuadVBO->Bind();
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(QuadVertex), (const void*)offsetof(QuadVertex, Position));

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(QuadVertex), (const void*)offsetof(QuadVertex, Color));

        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(QuadVertex), (const void*)offsetof(QuadVertex, TexCoord));

        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 1, GL_INT, GL_FALSE, sizeof(QuadVertex), (const void*)offsetof(QuadVertex, EntityID));

        uint32_t indices[s_Data.MaxIndices];
        uint32_t offset = 0;
        for (uint32_t i = 0; i < s_Data.MaxIndices; i += 6) {
            indices[i + 0] = 0 + offset;
            indices[i + 1] = 1 + offset;
            indices[i + 2] = 2 + offset;

            indices[i + 3] = 2 + offset;
            indices[i + 4] = 3 + offset;
            indices[i + 5] = 0 + offset;

            offset += 4;
        }
        Ref<IndexBuffer> squareIB;
        squareIB = IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t));
        s_Data.QuadVAO->SetIndexBuffer(squareIB);

        s_Data.CameraUniformBuffer = UniformBuffer::Create(sizeof(Renderer2DData::CameraData));
    }

    void Renderer2D::BeginScene(const glm::mat4& projection, const glm::mat4 &transform)
    {
        s_Data.CameraBuffer.ProjectionMatrix = projection;
        s_Data.CameraBuffer.ViewMatrix = glm::inverse(transform);
        s_Data.CameraUniformBuffer->SetData(&s_Data.CameraBuffer, sizeof(Renderer2DData::CameraData));
    }

    void Renderer2D::BeginScene(EditorCamera& camera)
    {
        s_Data.CameraBuffer.ProjectionMatrix = camera.GetProjection();
        s_Data.CameraBuffer.ViewMatrix = camera.GetViewMatrix();
        s_Data.CameraUniformBuffer->SetData(&s_Data.CameraBuffer, sizeof(Renderer2DData::CameraData));
    }

    void Renderer2D::EndScene()
    {
        Flush();
        s_Data.QuadVBO->ClearData();
    }

    void Renderer2D::BeginBatch()
    {
        s_Data.QuadVertexBufferPtr = s_Data.QuadVertexBufferBase;
    }

    void Renderer2D::EndBatch()
    {
        GLsizeiptr size = (uint8_t*)s_Data.QuadVertexBufferPtr - (uint8_t*)s_Data.QuadVertexBufferBase;
        s_Data.QuadVBO->SetData((float*)s_Data.QuadVertexBufferBase, size);
    }

    void Renderer2D::Flush()
    {
        if (s_Data.QuadIndexCount == 0)
            return;

        glm::mat4 modelMatrix = glm::mat4(1.0f);
        m_SceneData->Shader->Bind();
        std::dynamic_pointer_cast<OpenGLShader>(m_SceneData->Shader)->UploadUniformMat4("u_ModelMatrix", modelMatrix);

        s_Data.QuadVAO->Bind();
        RenderCommand::DrawIndexed(s_Data.QuadVAO);

        s_Data.Stats.DrawCalls++;
        s_Data.QuadIndexCount = 0;
        s_Data.TextureSlotIndex = 1;
    }

    void Renderer2D::DrawQuad(const glm::mat4 &transform, const glm::vec4 &color, uint32_t entityID)
    {
        if (s_Data.QuadIndexCount >= s_Data.MaxIndices)
        {
            EndBatch();
            Flush();
            BeginBatch();
        }

        constexpr size_t quadVertexCount = 4;
        for (size_t i = 0; i < quadVertexCount; i++)
        {
            s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[i];
            s_Data.QuadVertexBufferPtr->Color = color;
            s_Data.QuadVertexBufferPtr->TexCoord = s_Data.QuadTexCoords[i];
            s_Data.QuadVertexBufferPtr->EntityID = entityID;
            s_Data.QuadVertexBufferPtr++;
        }

        s_Data.QuadIndexCount += 6;
        s_Data.Stats.QuadCount++;
    }

    void Renderer2D::Submit(const Ref<Shader> &shader)
    {
        m_SceneData->Shader = shader;
    }

    Renderer2D::Statistics Renderer2D::GetStats()
    {
        return s_Data.Stats;
    }

    void Renderer2D::ResetStats()
    {
        s_Data.Stats.DrawCalls = 0;
        s_Data.Stats.QuadCount = 0;
    }
}
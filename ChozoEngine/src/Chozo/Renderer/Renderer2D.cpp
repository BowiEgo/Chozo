#include "Renderer2D.h"
#include "Renderer.h"

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
        float TexIndex;
        float TilingFactor;

        // Editor only
        int EntityID;
    };

    struct CircleVertex
    {
        glm::vec3 Position;
        glm::vec3 LocalPosition;
        glm::vec4 Color;
        float Thickness;
        float Fade;

        // Editor only
        int EntityID;
    };

    struct LineVertex
    {
        glm::vec3 Position;
        glm::vec4 Color;

        // Editor only
        int EntityID;
    };

    struct Renderer2DData
    {
        static const uint32_t MaxQuads = 20000;
        static const uint32_t MaxVertexs = MaxQuads * 4;
        static const uint32_t MaxIndices = MaxQuads * 6;
        int MaxTextureSlots = 0;

        Ref<VertexArray> QuadVertexArray;
        Ref<VertexBuffer> QuadVertexBuffer;
        Ref<Shader> TextureShader;

        Ref<VertexArray> CircleVertexArray;
        Ref<VertexBuffer> CircleVertexBuffer;
        Ref<Shader> CircleShader;

        Ref<VertexArray> LineVertexArray;
        Ref<VertexBuffer> LineVertexBuffer;
        Ref<Shader> LineShader;

        Ref<Texture2D> WhiteTexture;

        uint32_t QuadIndexCount = 0;
        QuadVertex* QuadVertexBufferBase = nullptr;
        QuadVertex* QuadVertexBufferPtr = nullptr;

        uint32_t CircleIndexCount = 0;
        CircleVertex* CircleVertexBufferBase = nullptr;
        CircleVertex* CircleVertexBufferPtr = nullptr;

        uint32_t LineVertexCount = 0;
        LineVertex* LineVertexBufferBase = nullptr;
        LineVertex* LineVertexBufferPtr = nullptr;

        float LineWidth = 2.0f;

        std::vector<Ref<Texture2D>> TextureSlots;
        uint32_t TextureSlotIndex = 1; // 0 = white texture

        glm::vec4 QuadVertexPositions[4] = {
            { -0.5f, -0.5f, 0.0f, 1.0f },
            {  0.5f, -0.5f, 0.0f, 1.0f },
            {  0.5f,  0.5f, 0.0f, 1.0f },
            { -0.5f,  0.5f, 0.0f, 1.0f }
        };

        glm::vec2 QuadTexCoords[4] = {
            { 0.0f, 0.0f },
            { 1.0f, 0.0f },
            { 1.0f, 1.0f },
            { 0.0f, 1.0f }
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

        // Quad
        s_Data.QuadVertexArray = VertexArray::Create();
        s_Data.QuadVertexBuffer = VertexBuffer::Create(s_Data.MaxVertexs * sizeof(QuadVertex));
        s_Data.QuadVertexBuffer->SetLayout({
            { ShaderDataType::Float3, "a_Position"    },
            { ShaderDataType::Float4, "a_Color"       },
            { ShaderDataType::Float2, "a_TexCoord"    },
            { ShaderDataType::Float,  "a_TexIndex"    },
            { ShaderDataType::Float,  "a_TilingFactor"},
            { ShaderDataType::Int,    "a_EntityID"    }
        });

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
        Ref<IndexBuffer> quadIndexBuffer;
        quadIndexBuffer = IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t));

        s_Data.QuadVertexArray->AddVertexBuffer(s_Data.QuadVertexBuffer);
        s_Data.QuadVertexArray->SetIndexBuffer(quadIndexBuffer);
        s_Data.QuadVertexBufferBase = new QuadVertex[s_Data.MaxVertexs];
        s_Data.QuadVertexBufferPtr = s_Data.QuadVertexBufferBase;

        // Circle
        s_Data.CircleVertexArray = VertexArray::Create();
        s_Data.CircleVertexBuffer = VertexBuffer::Create(s_Data.MaxVertexs * sizeof(CircleVertex));
        s_Data.CircleVertexBuffer->SetLayout({
            { ShaderDataType::Float3, "a_Position"      },
            { ShaderDataType::Float3, "a_LocalPosition" },
            { ShaderDataType::Float4, "a_Color"         },
            { ShaderDataType::Float,  "a_Thickness"     },
            { ShaderDataType::Float,  "a_Fade"          },
            { ShaderDataType::Int,    "a_EntityID"      }
        });
        s_Data.CircleVertexArray->AddVertexBuffer(s_Data.CircleVertexBuffer);
        s_Data.CircleVertexArray->SetIndexBuffer(quadIndexBuffer); // use quadIndexBuffer
        s_Data.CircleVertexBufferBase = new CircleVertex[s_Data.MaxVertexs];
        s_Data.CircleVertexBufferPtr = s_Data.CircleVertexBufferBase;

        // Line
        s_Data.LineVertexArray = VertexArray::Create();
        s_Data.LineVertexBuffer = VertexBuffer::Create(s_Data.MaxVertexs * sizeof(LineVertex));
        s_Data.LineVertexBuffer->SetLayout({
            { ShaderDataType::Float3, "a_Position" },
            { ShaderDataType::Float4, "a_Color"    },
            { ShaderDataType::Int,    "a_EntityID" }
        });
        s_Data.LineVertexArray->AddVertexBuffer(s_Data.LineVertexBuffer);
        s_Data.LineVertexBufferBase = new LineVertex[s_Data.MaxVertexs];
        s_Data.LineVertexBufferPtr = s_Data.LineVertexBufferBase;

        // Textures
        s_Data.MaxTextureSlots = RenderCommand::GetMaxTextureSlots();
        s_Data.TextureSlots.resize(s_Data.MaxTextureSlots);
        s_Data.WhiteTexture = Texture2D::Create();
        uint32_t whiteTextureData = 0xffffffff;
        s_Data.WhiteTexture->SetData(&whiteTextureData, sizeof(uint32_t));

        int32_t samplers[s_Data.MaxTextureSlots];
        for (uint32_t i = 0; i < s_Data.MaxTextureSlots; i++)
        {
            samplers[i] = i;
            s_Data.TextureSlots[i] = s_Data.WhiteTexture;
        }

        // Shader
        std::vector<int> samplersVec(samplers, samplers + s_Data.MaxTextureSlots);
        ShaderSpecification textureShaderSpec;
        textureShaderSpec.VertexFilepath = "../resources/shaders/Texture.glsl.vert";
        textureShaderSpec.FragmentFilepath = "../resources/shaders/Texture.glsl.frag";
        s_Data.TextureShader = Shader::Create(textureShaderSpec);
        s_Data.TextureShader->Bind();
        s_Data.TextureShader->SetUniform("u_Textures", samplersVec, s_Data.MaxTextureSlots);

        ShaderSpecification circleShaderSpec;
        circleShaderSpec.VertexFilepath = "../resources/shaders/Circle.glsl.vert";
        circleShaderSpec.FragmentFilepath = "../resources/shaders/Circle.glsl.frag";
        s_Data.CircleShader = Shader::Create(circleShaderSpec);

        ShaderSpecification lineShaderSpec;
        lineShaderSpec.VertexFilepath = "../resources/shaders/Line.glsl.vert";
        lineShaderSpec.FragmentFilepath = "../resources/shaders/Line.glsl.frag";
        s_Data.LineShader = Shader::Create(lineShaderSpec);

        // Uniform buffer
        // s_Data.CameraUniformBuffer = UniformBuffer::Create(sizeof(Renderer2DData::CameraData));
    }

    void Renderer2D::BeginScene(const glm::mat4& projection, const glm::mat4 &transform)
    {
        s_Data.CameraBuffer.ProjectionMatrix = projection;
        s_Data.CameraBuffer.ViewMatrix = glm::inverse(transform);
        // s_Data.CameraUniformBuffer->SetData(&s_Data.CameraBuffer, sizeof(Renderer2DData::CameraData));
    }

    void Renderer2D::BeginScene(EditorCamera& camera)
    {
        s_Data.CameraBuffer.ProjectionMatrix = camera.GetProjection();
        s_Data.CameraBuffer.ViewMatrix = camera.GetViewMatrix();
        // s_Data.CameraUniformBuffer->SetData(&s_Data.CameraBuffer, sizeof(Renderer2DData::CameraData));
    }

    void Renderer2D::EndScene()
    {
        Flush();
        s_Data.QuadVertexBuffer->ClearData();
        s_Data.CircleVertexBuffer->ClearData();
        s_Data.LineVertexBuffer->ClearData();
    }

    void Renderer2D::BeginBatch()
    {
        s_Data.QuadIndexCount = 0;
        s_Data.QuadVertexBufferPtr = s_Data.QuadVertexBufferBase;

        s_Data.CircleIndexCount = 0;
        s_Data.CircleVertexBufferPtr = s_Data.CircleVertexBufferBase;

        s_Data.LineVertexCount = 0;
        s_Data.LineVertexBufferPtr = s_Data.LineVertexBufferBase;

        s_Data.TextureSlotIndex = 1;
    }

    void Renderer2D::NextBatch()
    {
        Flush();
        BeginBatch();
    }

    void Renderer2D::Flush()
    {
        if (s_Data.QuadIndexCount)
        {
            GLsizeiptr size = (uint8_t*)s_Data.QuadVertexBufferPtr - (uint8_t*)s_Data.QuadVertexBufferBase;
            s_Data.QuadVertexBuffer->SetData(0, size, (float*)s_Data.QuadVertexBufferBase);

            for (uint32_t i = 0; i < s_Data.TextureSlots.size(); i++)
                s_Data.TextureSlots[i]->Bind(i);

            s_Data.TextureShader->Bind();
            RenderCommand::DrawIndexed(s_Data.QuadVertexArray, s_Data.QuadIndexCount);
            s_Data.Stats.DrawCalls++;
        }

        if (s_Data.CircleIndexCount)
        {
            GLsizeiptr size = (uint8_t*)s_Data.CircleVertexBufferPtr - (uint8_t*)s_Data.CircleVertexBufferBase;
            s_Data.CircleVertexBuffer->SetData(0, size, (float*)s_Data.CircleVertexBufferBase);

            s_Data.CircleShader->Bind();
            RenderCommand::DrawIndexed(s_Data.CircleVertexArray, s_Data.QuadIndexCount);
            s_Data.Stats.DrawCalls++;
        }

        if (s_Data.LineVertexCount)
        {
            GLsizeiptr size = (uint8_t*)s_Data.LineVertexBufferPtr - (uint8_t*)s_Data.LineVertexBufferBase;
            s_Data.LineVertexBuffer->SetData(0, size, (float*)s_Data.LineVertexBufferBase);

            s_Data.LineShader->Bind();
            RenderCommand::DrawLines(s_Data.LineVertexArray, s_Data.LineVertexCount);
            s_Data.Stats.DrawCalls++;
        }
    }

    void Renderer2D::DrawFullScreenQuad()
    {
        glm::mat4 transform = glm::scale(glm::mat4(1.0f), { 2.0f, 2.0f, 1.0f });

        constexpr size_t quadVertexCount = 4;
        for (size_t i = 0; i < quadVertexCount; i++)
        {
            s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[i];
            s_Data.QuadVertexBufferPtr->TexCoord = s_Data.QuadTexCoords[i];
            s_Data.QuadVertexBufferPtr++;
        }

        GLsizeiptr size = (uint8_t*)s_Data.QuadVertexBufferPtr - (uint8_t*)s_Data.QuadVertexBufferBase;
        s_Data.QuadVertexBuffer->SetData(0, size, (float*)s_Data.QuadVertexBufferBase);
        
        RenderCommand::DrawIndexed(s_Data.QuadVertexArray, 6);

        s_Data.QuadVertexBufferPtr = s_Data.QuadVertexBufferBase;
    }

    void Renderer2D::DrawFullScreenQuad(Ref<Texture> texture)
    {
        glm::mat4 transform = glm::scale(glm::mat4(1.0f), { 2.0f, 2.0f, 1.0f });

        constexpr size_t quadVertexCount = 4;
        for (size_t i = 0; i < quadVertexCount; i++)
        {
            s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[i];
            s_Data.QuadVertexBufferPtr->TexCoord = s_Data.QuadTexCoords[i];
            s_Data.QuadVertexBufferPtr++;
        }

        GLsizeiptr size = (uint8_t*)s_Data.QuadVertexBufferPtr - (uint8_t*)s_Data.QuadVertexBufferBase;
        s_Data.QuadVertexBuffer->SetData(0, size, (float*)s_Data.QuadVertexBufferBase);
        
        texture->Bind();
        Ref<Shader> shader = Renderer::GetRendererData().m_ShaderLibrary->Get("CubemapPreview");
        shader->Bind();
        shader->SetUniform("u_Texture", 0);
        RenderCommand::DrawIndexed(s_Data.QuadVertexArray, 6);

        s_Data.QuadVertexBufferPtr = s_Data.QuadVertexBufferBase;
    }

    void Renderer2D::DrawQuad(const glm::mat4 &transform, const glm::vec4 &color, uint32_t entityID)
    {
        if (s_Data.QuadIndexCount >= s_Data.MaxIndices)
        {
            NextBatch();
        }

        constexpr size_t quadVertexCount = 4;
        for (size_t i = 0; i < quadVertexCount; i++)
        {
            s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[i];
            s_Data.QuadVertexBufferPtr->Color = color;
            s_Data.QuadVertexBufferPtr->TexCoord = s_Data.QuadTexCoords[i];
            s_Data.QuadVertexBufferPtr->TexIndex = 0;
            s_Data.QuadVertexBufferPtr->TilingFactor = 0;
            s_Data.QuadVertexBufferPtr->EntityID = entityID;
            s_Data.QuadVertexBufferPtr++;
        }

        s_Data.QuadIndexCount += 6;
        s_Data.Stats.QuadCount++;
    }

    void Renderer2D::DrawQuad(const glm::mat4 &transform, Ref<Texture2D> texture, float tilingFactor, const glm::vec4 &color, uint32_t entityID)
    {
        uint32_t textureIndex = 0;

        for (uint32_t i = 0; i < s_Data.TextureSlots.size(); i++)
        {
            if (s_Data.TextureSlots[i] == texture)
            {
                textureIndex = i;
                break;
            }
        }

        if (textureIndex == 0)
        {
            textureIndex = s_Data.TextureSlotIndex;
            s_Data.TextureSlots[s_Data.TextureSlotIndex] = texture;
            s_Data.TextureSlotIndex++;
        }

        if (s_Data.QuadIndexCount >= s_Data.MaxIndices)
        {
            NextBatch();
        }

        constexpr size_t quadVertexCount = 4;
        for (size_t i = 0; i < quadVertexCount; i++)
        {
            s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[i];
            s_Data.QuadVertexBufferPtr->Color = color;
            s_Data.QuadVertexBufferPtr->TexCoord = s_Data.QuadTexCoords[i];
            s_Data.QuadVertexBufferPtr->TexIndex = textureIndex;
            s_Data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
            s_Data.QuadVertexBufferPtr->EntityID = entityID;
            s_Data.QuadVertexBufferPtr++;
        }

        s_Data.QuadIndexCount += 6;
        s_Data.Stats.QuadCount++;
    }

    void Renderer2D::DrawSprite(const glm::mat4 &transform, SpriteRendererComponent &src, uint32_t entityID)
    {
        if (src.Texture)
            DrawQuad(transform, src.Texture, src.TilingFactor, src.Color, entityID);
        else
            DrawQuad(transform, src.Color, entityID);
    }

    void Renderer2D::DrawCircle(const glm::mat4 &transform, const glm::vec4 &color, float thickness, float fade, uint32_t entityID)
    {
        for (size_t i = 0; i < 4; i++)
        {
            s_Data.CircleVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[i];
            s_Data.CircleVertexBufferPtr->LocalPosition = s_Data.QuadVertexPositions[i] * 2.0f;
            s_Data.CircleVertexBufferPtr->Color = color;
            s_Data.CircleVertexBufferPtr->Thickness = thickness;
            s_Data.CircleVertexBufferPtr->Fade = fade;
            s_Data.CircleVertexBufferPtr->EntityID = entityID;
            s_Data.CircleVertexBufferPtr++;
        }

        s_Data.CircleIndexCount += 6;
        s_Data.Stats.CircleCount++;
    }

    void Renderer2D::DrawLine(const glm::vec3 &p0, const glm::vec3 &p1, const glm::vec4 &color, uint32_t entityID)
    {
        glm::vec3 lineDir = glm::normalize(p1 - p0);
        glm::vec3 cameraDir = glm::normalize(glm::vec3(s_Data.CameraBuffer.ViewMatrix[0][2], s_Data.CameraBuffer.ViewMatrix[1][2], s_Data.CameraBuffer.ViewMatrix[2][2]));
        glm::vec3 normal = glm::normalize(glm::cross(lineDir, cameraDir));
        glm::vec3 offset = normal * s_Data.LineWidth * 0.005f;

        glm::vec3 vertices[4] = {
            p0 + offset,
            p0 - offset,
            p1 + offset,
            p1 - offset
        };

        for (size_t i = 0; i < 4; i++)
        {
            s_Data.LineVertexBufferPtr->Position = vertices[i];
            s_Data.LineVertexBufferPtr->Color = color;
            s_Data.LineVertexBufferPtr->EntityID = entityID;
            s_Data.LineVertexBufferPtr++;
        }

        s_Data.LineVertexCount += 4;
        s_Data.Stats.LineCount++;
    }

    void Renderer2D::DrawRect(const glm::vec3 &position, const glm::vec2 &size, const glm::vec4 &color, uint32_t entityID)
    {
        glm::vec3 p0 = glm::vec3(position.x - size.x * 0.5f, position.y - size.y * 0.5f, position.z);
        glm::vec3 p1 = glm::vec3(position.x + size.x * 0.5f, position.y - size.y * 0.5f, position.z);
        glm::vec3 p2 = glm::vec3(position.x + size.x * 0.5f, position.y + size.y * 0.5f, position.z);
        glm::vec3 p3 = glm::vec3(position.x - size.x * 0.5f, position.y + size.y * 0.5f, position.z);

        glm::vec3 cornerAdjust;

        cornerAdjust = glm::normalize(p0 - p1) * s_Data.LineWidth * 0.005f;
        DrawLine(p0 + cornerAdjust, p1 - cornerAdjust, color);
        NextBatch();
        cornerAdjust = glm::normalize(p1 - p2) * s_Data.LineWidth * 0.005f;
        DrawLine(p1 + cornerAdjust, p2 - cornerAdjust, color);
        NextBatch();
        cornerAdjust = glm::normalize(p2 - p3) * s_Data.LineWidth * 0.005f;
        DrawLine(p2 + cornerAdjust, p3 - cornerAdjust, color);
        NextBatch();
        cornerAdjust = glm::normalize(p3 - p0) * s_Data.LineWidth * 0.005f;
        DrawLine(p3 + cornerAdjust, p0 - cornerAdjust, color);
        NextBatch();
    }

    void Renderer2D::DrawRect(const glm::mat4 &transform, const glm::vec4 &color, uint32_t entityID)
    {
        glm::vec3 lineVertices[4];
        for (size_t i = 0; i < 4; i++)
            lineVertices[i] = transform * s_Data.QuadVertexPositions[i];

        glm::vec3 cornerAdjust;

        cornerAdjust = glm::normalize(lineVertices[0] - lineVertices[1]) * s_Data.LineWidth * 0.005f;
        DrawLine(lineVertices[0] + cornerAdjust, lineVertices[1] - cornerAdjust, color);
        NextBatch();
        cornerAdjust = glm::normalize(lineVertices[1] - lineVertices[2]) * s_Data.LineWidth * 0.005f;
        DrawLine(lineVertices[1] + cornerAdjust, lineVertices[2] - cornerAdjust, color);
        NextBatch();
        cornerAdjust = glm::normalize(lineVertices[2] - lineVertices[3]) * s_Data.LineWidth * 0.005f;
        DrawLine(lineVertices[2] + cornerAdjust, lineVertices[3] - cornerAdjust, color);
        NextBatch();
        cornerAdjust = glm::normalize(lineVertices[3] - lineVertices[0]) * s_Data.LineWidth * 0.005f;
        DrawLine(lineVertices[3] + cornerAdjust, lineVertices[0] - cornerAdjust, color);
        NextBatch();
    }

    float Renderer2D::GetLineWidth()
    {
        return s_Data.LineWidth;
    }

    void Renderer2D::SetLineWidth(float width)
    {
        s_Data.LineWidth = width;
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
        s_Data.Stats.LineCount = 0;
        s_Data.Stats.CircleCount = 0;
    }
}
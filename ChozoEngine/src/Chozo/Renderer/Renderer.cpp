#include "Renderer.h"

#include "Platform/OpenGL/OpenGLShader.h"
#include "UniformBuffer.h"

#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>

namespace Chozo {
    
    namespace FileSystem {

        static std::string ReadFile(const std::string &filepath)
        {
            std::string result;
            std::ifstream in(filepath, std::ios::in | std::ios::binary);
            if (in)
            {
                in.seekg(0, std::ios::end);
                result.resize(in.tellg());
                in.seekg(0, std::ios::beg);
                in.read(&result[0], result.size());
                in.close();
            }
            else
            {
                CZ_CORE_ASSERT("Could not open file '{0}'", filepath);
            }

            return result;
        }
    }

    struct RendererData
    {
        static const uint32_t MaxQuads = 20000;
        static const uint32_t MaxVertexs = MaxQuads * 4;
        static const uint32_t MaxIndices = MaxQuads * 6;
        static const uint32_t MaxIndexs = MaxQuads * 2;
        int MaxTextureSlots = 0;

        Ref<VertexArray> VertexArray;
        Ref<VertexBuffer> VertexBuffer;
        Ref<IndexBuffer> IndexBuffer;
        Ref<Shader> Shader;

        Ref<Texture2D> WhiteTexture;

        uint32_t VertexCount = 0;
        uint32_t IndexCount = 0;
        Vertex* VertexBufferBase = nullptr;
        Vertex* VertexBufferPtr = nullptr;

        Index* IndexBufferBase = nullptr;
        Index* IndexBufferPtr = nullptr;

        std::vector<Ref<Texture2D>> TextureSlots;
        uint32_t TextureSlotIndex = 1; // 0 = white texture

        struct CameraData
        {
            glm::mat4 ProjectionMatrix;
            glm::mat4 ViewMatrix;
        };
        CameraData CameraBuffer;
        Ref<UniformBuffer> CameraUniformBuffer;

        Renderer::Statistics Stats;
    };

    static RendererData s_Data;

    Renderer::SceneData* Renderer::m_SceneData = new Renderer::SceneData;

    void Renderer::Init()
    {
        RenderCommand::Init();

        s_Data.VertexArray = VertexArray::Create();
        s_Data.VertexBuffer = VertexBuffer::Create(s_Data.MaxVertexs * sizeof(Vertex));
        s_Data.VertexBuffer->SetLayout({
            { ShaderDataType::Float3, "a_Position" },
            { ShaderDataType::Float3, "a_Normal"   },
            { ShaderDataType::Float2, "a_TexCoord" },
            { ShaderDataType::Float3, "a_Tangent"  },
            { ShaderDataType::Float3, "a_Binormal" },
            { ShaderDataType::Int,    "a_EntityID" }
        });
        uint32_t indices[s_Data.MaxIndices];
        s_Data.IndexBuffer = IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t));

        s_Data.VertexArray->AddVertexBuffer(s_Data.VertexBuffer);
        s_Data.VertexArray->SetIndexBuffer(s_Data.IndexBuffer);
        s_Data.VertexBufferBase = new Vertex[s_Data.MaxVertexs];
        s_Data.IndexBufferBase = new Index[s_Data.MaxIndexs];

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
        ShaderSpecification shaderSpec;
        shaderSpec.VertexFilepath = "../assets/shaders/Shader.glsl.vert";
        shaderSpec.FragmentFilepath = "../assets/shaders/Shader.glsl.frag";
        s_Data.Shader = Shader::Create(shaderSpec);
        s_Data.Shader->Bind();
        s_Data.Shader->UploadUniformIntArray("u_Textures", samplers, s_Data.MaxTextureSlots);

        // Uniform buffer
        s_Data.CameraUniformBuffer = UniformBuffer::Create(sizeof(RendererData::CameraData));
    }

    void Renderer::BeginScene(const glm::mat4& projection, const glm::mat4 &transform)
    {
        s_Data.CameraBuffer.ProjectionMatrix = projection;
        s_Data.CameraBuffer.ViewMatrix = glm::inverse(transform);
        s_Data.CameraUniformBuffer->SetData(&s_Data.CameraBuffer, sizeof(RendererData::CameraData));
    }

    void Renderer::BeginScene(EditorCamera& camera)
    {
        s_Data.CameraBuffer.ProjectionMatrix = camera.GetProjection();
        s_Data.CameraBuffer.ViewMatrix = camera.GetViewMatrix();
        s_Data.CameraUniformBuffer->SetData(&s_Data.CameraBuffer, sizeof(RendererData::CameraData));
    }

    void Renderer::EndScene()
    {
        Flush();
        s_Data.VertexBuffer->ClearData();
        s_Data.IndexBuffer->ClearData();
    }

    void Renderer::BeginBatch()
    {
        s_Data.IndexCount = 0;
        s_Data.VertexCount = 0;
        s_Data.VertexBufferPtr = s_Data.VertexBufferBase;
        s_Data.IndexBufferPtr = s_Data.IndexBufferBase;
    }

    void Renderer::NextBatch()
    {
        Flush();
        BeginBatch();
    }

    void Renderer::Flush()
    {
        
        GLsizeiptr vertexBufferSize = (uint8_t*)s_Data.VertexBufferPtr - (uint8_t*)s_Data.VertexBufferBase;
        s_Data.VertexBuffer->SetData((float*)s_Data.VertexBufferBase, vertexBufferSize);

        GLsizeiptr indexBufferSize = (uint8_t*)s_Data.IndexBufferPtr - (uint8_t*)s_Data.IndexBufferBase;
        s_Data.IndexBuffer->SetData((unsigned int*)s_Data.IndexBufferBase, indexBufferSize);

        // for (uint32_t i = 0; i < s_Data.TextureSlots.size(); i++)
        //     s_Data.TextureSlots[i]->Bind(i);

        s_Data.Shader->Bind();
        RenderCommand::DrawIndexed(s_Data.VertexArray, s_Data.IndexCount);
        s_Data.Stats.DrawCalls++;
    }

    void Renderer::DrawMesh(const glm::mat4 transform, Mesh& mesh, uint32_t entityID)
    {
        std::vector<Vertex> vertices = mesh.GetVertices();
        for (int i = 0; i < vertices.size(); i++)
        {
            s_Data.VertexBufferPtr->Position = transform
                    * glm::vec4(vertices[i].Position.x,
                                vertices[i].Position.y,
                                vertices[i].Position.z,
                                1.0f);
            s_Data.VertexBufferPtr->Normal = vertices[i].Normal;
            s_Data.VertexBufferPtr->TexCoord = vertices[i].TexCoord;
            s_Data.VertexBufferPtr->Tangent = vertices[i].Tangent;
            s_Data.VertexBufferPtr->Binormal = vertices[i].Binormal;
            s_Data.VertexBufferPtr->EntityID = entityID;
            s_Data.VertexBufferPtr++;
        }

        std::vector<Index> indices = mesh.GetIndices();
        for (int i = 0; i < indices.size(); i++)
        {
            s_Data.IndexBufferPtr->V1 = indices[i].V1 + s_Data.VertexCount;
            s_Data.IndexBufferPtr->V2 = indices[i].V2 + s_Data.VertexCount;
            s_Data.IndexBufferPtr->V3 = indices[i].V3 + s_Data.VertexCount;
            s_Data.IndexBufferPtr++;
        }

        s_Data.Stats.triangleCount += mesh.GetTriangleCount();
        s_Data.VertexCount += vertices.size();
        s_Data.IndexCount += mesh.GetIndexCount();
    }

    Renderer::Statistics Renderer::GetStats()
    {
        return s_Data.Stats;
    }

    void Renderer::ResetStats()
    {
        s_Data.Stats.DrawCalls = 0;
    }
}
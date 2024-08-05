#include "Mesh.h"

#include "Renderer.h"

namespace Chozo
{

    Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<Index>& indices, const uint32_t& indexCount, const uint32_t& triangleCount)
        : m_Vertices(vertices), m_Indices(indices), m_IndexCount(indexCount), m_TriangleCount(triangleCount)
    {
        // m_VertexArray = VertexArray::Create();
        // m_VertexBuffer = VertexBuffer::Create(m_Vertices.data(), (uint32_t)(m_Vertices.size() * sizeof(Vertex)));
        // m_VertexBuffer->SetLayout({
        //     { ShaderDataType::Float3, "a_Position" },
        //     { ShaderDataType::Float3, "a_Normal"   },
        //     { ShaderDataType::Float2, "a_TexCoord" },
        //     { ShaderDataType::Float3, "a_Tangent"  },
        //     { ShaderDataType::Float3, "a_Binormal" },
        //     { ShaderDataType::Int,    "a_EntityID" }
        // });
		// m_IndexBuffer = IndexBuffer::Create(m_Indices.data(), (uint32_t)(m_Indices.size() * sizeof(Index)));

        // m_VertexArray->AddVertexBuffer(m_VertexBuffer);
        // m_VertexArray->SetIndexBuffer(m_IndexBuffer);
    }
}

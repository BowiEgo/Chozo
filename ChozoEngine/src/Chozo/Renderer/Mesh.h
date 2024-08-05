#pragma once

#include "czpch.h"

#include "VertexArray.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"

#include <glm/glm.hpp>

namespace Chozo
{

    struct Vertex
	{
		glm::vec3 Position;
		glm::vec3 Normal;
		glm::vec2 TexCoord;
		glm::vec3 Tangent;
		glm::vec3 Binormal;

        // Editor only
        int EntityID;
	};

    struct Index
	{
		uint32_t V1, V2, V3;
	};

    //TODO: Static Mesh, Instanced Mesh, Dynamic Mesh
    class Mesh
    {
    public:
        Mesh() = default;
        Mesh(const std::vector<Vertex>& vertices, const std::vector<Index>& indices, const uint32_t& indexCount, const uint32_t& triangleCount);

        std::vector<Vertex> GetVertices() { return m_Vertices; }
        std::vector<Index> GetIndices() { return m_Indices; }
        Ref<VertexArray> GetVertexArray() { return m_VertexArray; }
        uint32_t GetIndexCount() { return m_IndexCount; }
        uint32_t GetTriangleCount() { return m_TriangleCount; }

        ~Mesh() = default;
    protected:
		Ref<VertexArray> m_VertexArray;
		Ref<VertexBuffer> m_VertexBuffer;
        Ref<IndexBuffer> m_IndexBuffer;

		std::vector<Vertex> m_Vertices;
		std::vector<Index> m_Indices;
        uint32_t m_IndexCount = 0, m_TriangleCount = 0;
    };
}

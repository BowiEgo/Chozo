#pragma once

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
	};

    struct Index
	{
		uint32_t V1, V2, V3;
	};

    struct Triangle
	{
		Vertex V0, V1, V2;

		Triangle(const Vertex& v0, const Vertex& v1, const Vertex& v2)
			: V0(v0), V1(v1), V2(v2) {}
	};
	
#ifdef CZ_OUTPUT_VALUE
	std::ostream& operator<<(std::ostream& os, const Vertex& vertex)
    {
        os << "Position:" << "{" << vertex.Position.x << ", " << vertex.Position.y << ", " << vertex.Position.z << "}";
        return os;
    }

	std::ostream& operator<<(std::ostream& os, const Index& index)
    {
        os << "{" << index.V1 << ", " << index.V2 << ", " << index.V3 << "}";
        return os;
    }

	std::cout << "SubmitMesh: ";
	for (uint32_t i = 0; i < meshSource->GetIndexs().size(); ++i) {
	    std::cout << meshSource->GetIndexs().data()[i] << " ";
	}
	std::cout << std::endl;
#endif

	struct RenderSource : public RefCounted
    {
        Ref<VertexArray> VAO;
        Ref<VertexBuffer> VBO;
        Ref<IndexBuffer> IBO;

		RenderSource() = default;
        RenderSource(const RenderSource& other) = delete;
        // {
        // }
		RenderSource(uint32_t vertexCount, uint32_t indexCount)
		{
			VAO = VertexArray::Create();
            VBO = VertexBuffer::Create(vertexCount * sizeof(Vertex));
            VBO->SetLayout({
                { ShaderDataType::Float3, "a_Position" },
                { ShaderDataType::Float3, "a_Normal"   },
                { ShaderDataType::Float2, "a_TexCoord" },
                { ShaderDataType::Float3, "a_Tangent"  },
                { ShaderDataType::Float3, "a_Binormal" },
            });
            uint32_t maxIndicesCount = indexCount * 3;
            uint32_t indices[maxIndicesCount];
            IBO = IndexBuffer::Create(indices, maxIndicesCount);

            VAO->AddVertexBuffer(VBO);
            VAO->SetIndexBuffer(IBO);
		};
    };
}

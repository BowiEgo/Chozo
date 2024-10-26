#pragma once

#include "czpch.h"

#include "Chozo/Asset/Asset.h"

#include "VertexArray.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "Batch.h"
#include "DataStructs.h"

#include "Chozo/Math/AABB.h"

#include <glm/glm.hpp>

namespace Chozo
{
    struct MeshBuffer
    {
        std::vector<Vertex> Vertexs;
		std::vector<Index> Indexs;
        uint32_t IndexCount = 0, IndicesCount = 0;
    };

    enum class MeshType
    {
        Dynamic, Instanced, Static
    };

    class Mesh;

    class Submesh
	{
	public:
		uint32_t BaseVertex;
		uint32_t BaseIndex;
		uint32_t MaterialIndex;
		uint32_t IndexCount;
		uint32_t VertexCount;

		glm::mat4 Transform{ 1.0f }; // World transform
		glm::mat4 LocalTransform{ 1.0f };
		AABB BoundingBox;

		std::string NodeName, MeshName;
		bool IsRigged = false;
	};

    struct MeshNode
	{
		uint32_t Parent = 0xffffffff;
		std::vector<uint32_t> Children;
		std::vector<uint32_t> Submeshes;

		std::string Name;
		glm::mat4 LocalTransform;

		inline bool IsRoot() const { return Parent == 0xffffffff; }
	};

    class MeshSource : public Asset
    {
    public:
        MeshSource() = default;
        MeshSource(const std::string path);
        MeshSource(const std::vector<Vertex>& vertexs, const std::vector<Index>& indexs, const uint32_t& indexCount, const uint32_t& indicesCount);
        virtual ~MeshSource() = default;

        static Ref<MeshSource> Create(const std::string &path);

        static AssetType GetStaticType() { return AssetType::MeshSource; }
		virtual AssetType GetAssetType() const override { return GetStaticType(); }

        inline std::vector<Vertex> GetVertexs() { return m_Buffer.Vertexs; }
        inline std::vector<Index> GetIndexs() { return m_Buffer.Indexs; }
        inline uint32_t GetTriangleCount() { return m_Buffer.IndexCount; }

        inline MeshBuffer* GetBuffer() { return &m_Buffer; }

        std::vector<Submesh>& GetSubmeshes() { return m_Submeshes; }
		const std::vector<Submesh>& GetSubmeshes() const { return m_Submeshes; }
        
		const MeshNode& GetRootNode() const { return m_Nodes[0]; }
		const std::vector<MeshNode>& GetNodes() const { return m_Nodes; }
    protected:
        MeshBuffer m_Buffer;
    private:
        AABB m_BoundingBox;
		std::vector<Submesh> m_Submeshes;
		std::unordered_map<uint32_t, std::vector<Triangle>> m_TriangleCache;
		std::vector<MeshNode> m_Nodes;

        friend class MeshImporter;
        friend class Geometry;
    };

    class Mesh : public RefCounted
    {
    public:
        Mesh();
        Mesh(Ref<MeshSource>& meshSource)
            : m_MeshSource(meshSource)
        {
            Invalidate();
        }
        virtual ~Mesh() = default;

        void Invalidate();

        inline Ref<VertexArray> GetVertexArray() const { return m_RenderSource->VAO; }
        inline Ref<VertexBuffer> GetVertexBuffer() const { return m_RenderSource->VBO; }
        inline Ref<IndexBuffer> GetIndexBuffer() const { return m_RenderSource->IBO; }

        Ref<MeshSource> GetMeshSource() const { return m_MeshSource; }

        operator bool() { return m_MeshSource->GetBuffer()->IndexCount != 0; }
    protected:
        Ref<MeshSource> m_MeshSource;
        Ref<RenderSource> m_RenderSource;
    };

    class DynamicMesh : public Mesh
    {
    public:
        DynamicMesh() = default;
        DynamicMesh(Ref<MeshSource> meshSource);
        ~DynamicMesh() {};
    };

    class InstancedMesh : public Mesh
    {
    public:
        InstancedMesh() = default;
        InstancedMesh(Ref<MeshSource> meshSource);
        ~InstancedMesh() = default;
    };

    class StaticMesh : public Mesh
    {
    public:
        StaticMesh() = default;
        StaticMesh(Ref<MeshSource>& meshSource)
            : Mesh(meshSource) {}
        ~StaticMesh() { CallRemove(); };

        UUID GetBufferSegmentID() const { return m_BufferSegmentID; }
        void SetBufferSegmentID(UUID& id) { m_BufferSegmentID = id; }

        void OnSubmit(bool successed);
        void CallRemove();
    private:
        UUID m_BufferSegmentID;
    };
}

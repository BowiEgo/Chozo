#pragma once

#include "czpch.h"

#include "VertexArray.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "Batch.h"
#include "DataStructs.h"

#include "Material.h"

#include "Chozo/Math/AABB.h"
#include "Chozo/Asset/Asset.h"
#include "Chozo/FileSystem/StreamWriter.h"
#include "Chozo/FileSystem/StreamReader.h"

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

        static void Serialize(StreamWriter* serializer, const Submesh& instance)
		{
			serializer->WriteRaw(instance.BaseVertex);
			serializer->WriteRaw(instance.BaseIndex);
			serializer->WriteRaw(instance.MaterialIndex);
			serializer->WriteRaw(instance.IndexCount);
			serializer->WriteRaw(instance.VertexCount);
			serializer->WriteRaw(instance.Transform);
			serializer->WriteRaw(instance.LocalTransform);
			serializer->WriteRaw(instance.BoundingBox);
			serializer->WriteString(instance.NodeName);
			serializer->WriteString(instance.MeshName);
			serializer->WriteRaw(instance.IsRigged);
		}

		static void Deserialize(StreamReader* deserializer, Submesh& instance)
		{
			deserializer->ReadRaw(instance.BaseVertex);
			deserializer->ReadRaw(instance.BaseIndex);
			deserializer->ReadRaw(instance.MaterialIndex);
			deserializer->ReadRaw(instance.IndexCount);
			deserializer->ReadRaw(instance.VertexCount);
			deserializer->ReadRaw(instance.Transform);
			deserializer->ReadRaw(instance.LocalTransform);
			deserializer->ReadRaw(instance.BoundingBox);
			deserializer->ReadString(instance.NodeName);
			deserializer->ReadString(instance.MeshName);
			deserializer->ReadRaw(instance.IsRigged);
		}
	};

    struct MeshNode
	{
		uint32_t Parent = 0xffffffff;
		std::vector<uint32_t> Children;
		std::vector<uint32_t> Submeshes;

		std::string Name;
		glm::mat4 LocalTransform;

		inline bool IsRoot() const { return Parent == 0xffffffff; }

        static void Serialize(StreamWriter* serializer, const MeshNode& instance)
		{
			serializer->WriteRaw(instance.Parent);
			serializer->WriteArray(instance.Children);
			serializer->WriteArray(instance.Submeshes);
			serializer->WriteString(instance.Name);
			serializer->WriteRaw(instance.LocalTransform);
		}

		static void Deserialize(StreamReader* deserializer, MeshNode& instance)
		{
			deserializer->ReadRaw(instance.Parent);
			deserializer->ReadArray(instance.Children);
			deserializer->ReadArray(instance.Submeshes);
			deserializer->ReadString(instance.Name);
			deserializer->ReadRaw(instance.LocalTransform);
		}
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

        const inline AABB& GetBoundingBox() const { return m_BoundingBox; }

        inline std::vector<Vertex> GetVertexs() { return m_Buffer.Vertexs; }
        inline std::vector<Index> GetIndexs() { return m_Buffer.Indexs; }
        inline uint32_t GetTriangleCount() { return m_Buffer.IndexCount; }

        inline MeshBuffer* GetBuffer() { return &m_Buffer; }

        std::vector<Submesh>& GetSubmeshes() { return m_Submeshes; }
		const std::vector<Submesh>& GetSubmeshes() const { return m_Submeshes; }

        const std::vector<AssetHandle>& GetMaterials() { return m_Materials; }
        
		const MeshNode& GetRootNode() const { return m_Nodes[0]; }
		const std::vector<MeshNode>& GetNodes() const { return m_Nodes; }
    protected:
        MeshBuffer m_Buffer;
    private:
        AABB m_BoundingBox;
		std::vector<Submesh> m_Submeshes;
		std::unordered_map<uint32_t, std::vector<Triangle>> m_TriangleCache;
		std::vector<MeshNode> m_Nodes;

		std::vector<AssetHandle> m_Materials;

        friend class MeshImporter;
        friend class MeshSourceSerializer;
        friend class Geometry;
    };

    class Mesh : public RefCounted
    {
    public:
    	using OnChangeFunc = std::function<void()>;

        Mesh();
        Mesh(Ref<MeshSource>& meshSource)
            : m_MeshSource(meshSource)
        {
            Invalidate();
        }
        virtual ~Mesh() = default;

        void Invalidate();

        Ref<VertexArray> GetVertexArray() const { return m_RenderSource->VAO; }
        Ref<VertexBuffer> GetVertexBuffer() const { return m_RenderSource->VBO; }
        Ref<IndexBuffer> GetIndexBuffer() const { return m_RenderSource->IBO; }

        Ref<MeshSource> GetMeshSource() const { return m_MeshSource; }

        void SetMaterial(uint32_t submeshIndex, const AssetHandle handle)
        {
            m_MeshSource->GetSubmeshes()[submeshIndex].MaterialIndex = m_Materials->SetMaterial(handle);
        }
        Ref<MaterialTable> GetMaterials() { return m_Materials; }

        operator bool() { return m_MeshSource->GetBuffer()->IndexCount != 0; }

    	void RegisterOnChange(const OnChangeFunc &callback) { m_OnChangeCbs.push_back(callback); }
    	void NotifyChange() const {
        	for (const auto& callback : m_OnChangeCbs)
        	{
        		if (callback)
        			callback();
        	}
        }
    protected:
        Ref<MeshSource> m_MeshSource;
        Ref<RenderSource> m_RenderSource;
        Ref<MaterialTable> m_Materials;
    	std::vector<OnChangeFunc> m_OnChangeCbs;
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

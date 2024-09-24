#pragma once

#include "czpch.h"

#include "VertexArray.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "Batch.h"

#include "DataStructs.h"
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

    class MeshSource : public RefCounted
    {
    public:
        MeshSource() = default;
        MeshSource(const std::vector<Vertex>& vertexs, const std::vector<Index>& indexs, const uint32_t& indexCount, const uint32_t& indicesCount);
        virtual ~MeshSource() = default;

        inline std::vector<Vertex> GetVertexs() { return m_Buffer.Vertexs; }
        inline std::vector<Index> GetIndexs() { return m_Buffer.Indexs; }
        inline uint32_t GetTriangleCount() { return m_Buffer.IndexCount; }

        inline glm::mat4 GetLocalTransform() { return m_LocalTransform; }
        void SetLocalTransform(const glm::mat4& transform);

        inline MeshBuffer* GetBuffer() { return &m_Buffer; }
        inline MeshBuffer* GetTempBuffer() { return &m_TempBuffer; }
        inline bool IsBufferChanged() { return m_Is_Buffer_Changed; }
        inline void SetBufferChanged(bool changed) { m_Is_Buffer_Changed = changed; }

        inline uint64_t GetEntityID() const { return m_EntityID - 1; }
        inline void SetEntityID(uint64_t id) { m_EntityID = id + 1; }

        void CallGenerate();
        void AfterGenerate(bool successed);
    protected:
        virtual void Backup() = 0;
        virtual void Backtrace() = 0;
        virtual MeshBuffer* Generate() = 0;
    protected:
        MeshBuffer m_Buffer;
        glm::mat4 m_LocalTransform{1.0f};
        uint64_t m_EntityID;
    private:
        MeshBuffer m_TempBuffer;
        bool m_Is_Buffer_Changed = false;
    };

    class Mesh : public RefCounted
    {
    public:
        Mesh() = default;
        Mesh(Ref<MeshSource>& meshSource)
            : m_MeshSource(meshSource) {}
        virtual ~Mesh() = default;

        Ref<MeshSource> GetMeshSource() const { return m_MeshSource; }
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

        void Init();

        inline Ref<VertexArray> GetVertexArray() const { return m_RenderSource->VAO; }
        inline Ref<VertexBuffer> GetVertexBuffer() const { return m_RenderSource->VBO; }
        inline Ref<IndexBuffer> GetIndexBuffer() const { return m_RenderSource->IBO; }
    };

    class InstancedMesh : public Mesh
    {
    public:
        InstancedMesh() = default;
        InstancedMesh(Ref<MeshSource> meshSource);
        ~InstancedMesh() = default;

        inline Ref<VertexArray> GetVertexArray() { return m_RenderSource->VAO; }
        inline Ref<VertexBuffer> GetVertexBuffer() { return m_RenderSource->VBO; }
        inline Ref<IndexBuffer> GetIndexBuffer() { return m_RenderSource->IBO; }
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

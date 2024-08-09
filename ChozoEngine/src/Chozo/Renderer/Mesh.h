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

    class Mesh;

    class MeshSource
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

        void AfterGenerate(bool successed);
    protected:
        virtual void Backup() = 0;
        virtual void Backtrace() = 0;
        virtual MeshBuffer* Generate() = 0;
        void CallGenerate();
    protected:
        MeshBuffer m_Buffer;
        glm::mat4 m_LocalTransform{1.0f};
    private:
        MeshBuffer m_TempBuffer;
        bool m_Is_Buffer_Changed = false;
    };

    //TODO: Instanced Mesh, Dynamic Mesh
    class Mesh
    {
    public:
        Mesh() = default;
        Mesh(Ref<MeshSource>& meshSource)
            : m_MeshSource(meshSource) {}
        ~Mesh() = default;

        Ref<MeshSource> GetMeshSource() { return m_MeshSource; }
    protected:
        Ref<MeshSource> m_MeshSource;
    };

    class StaticMesh : public Mesh
    {
    public:
        StaticMesh() = default;
        StaticMesh(Ref<MeshSource>& meshSource)
            : Mesh(meshSource) {}
        ~StaticMesh();

        UUID GetBufferSegmentID() const { return m_BufferSegmentID; }
        void SetBufferSegmentID(UUID& id) { m_BufferSegmentID = id; }

        void CallSubmit();
        void CallRemove();
    private:
        UUID m_BufferSegmentID;
    };

    //TODO: Instanced Mesh, Dynamic Mesh
    class InstanceMesh : public Mesh
    {
    public:
        InstanceMesh() = default;
        InstanceMesh(Ref<MeshSource>& meshSource);
        ~InstanceMesh() = default;

        inline Ref<VertexArray> GetVertexArray() { return m_VertexArray; }
    private:
        Ref<VertexArray> m_VertexArray;
		Ref<VertexBuffer> m_VertexBuffer;
        Ref<IndexBuffer> m_IndexBuffer;
    };

    class DynamicMesh : public Mesh
    {
    public:
        DynamicMesh() = default;
        DynamicMesh(Ref<MeshSource>& meshSource);
        ~DynamicMesh() = default;

        inline Ref<VertexArray> GetVertexArray() { return m_VertexArray; }
    private:
        Ref<VertexArray> m_VertexArray;
		Ref<VertexBuffer> m_VertexBuffer;
        Ref<IndexBuffer> m_IndexBuffer;
    };

}

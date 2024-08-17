#include "Mesh.h"

#include "Renderer.h"

namespace Chozo
{

    //////////////////////////////////////////////////////////
    //---------------------Mesh Source----------------------//
    //////////////////////////////////////////////////////////
    MeshSource::MeshSource(const std::vector<Vertex>& vertexs, const std::vector<Index>& indexs, const uint32_t& indexCount, const uint32_t& indicesCount)
    {
        m_Buffer.Vertexs = vertexs;
        m_Buffer.Indexs = indexs;
        m_Buffer.IndexCount = indexCount;
        m_Buffer.IndicesCount = indicesCount;
    }

    void MeshSource::SetLocalTransform(const glm::mat4 &transform)
    {
        if (m_LocalTransform == transform)
            return;

        m_LocalTransform = transform;
        CallGenerate();
    }

    void MeshSource::CallGenerate()
    {
        MeshBuffer* buffer = Generate();
        m_TempBuffer.Vertexs = buffer->Vertexs;
        m_TempBuffer.Indexs = buffer->Indexs;
        m_TempBuffer.IndexCount = buffer->IndexCount;
        m_TempBuffer.IndicesCount = buffer->IndicesCount;

        SetBufferChanged(true);
        delete buffer;
    }

    void MeshSource::AfterGenerate(bool successed)
    {
        if (successed)
        {
            m_Buffer.Vertexs = m_TempBuffer.Vertexs;
            m_Buffer.Indexs = m_TempBuffer.Indexs;
            m_Buffer.IndexCount = m_TempBuffer.IndexCount;
            m_Buffer.IndicesCount = m_TempBuffer.IndicesCount;
        }
        else
            Backtrace();

        SetBufferChanged(false);
    }

    StaticMesh::~StaticMesh()
    {
    }

    //////////////////////////////////////////////////////////
    //---------------------Static Mesh----------------------//
    //////////////////////////////////////////////////////////
    void StaticMesh::CallSubmit()
    {
        bool successed = Renderer::SubmitMesh(this);
        m_MeshSource->AfterGenerate(successed);
    }

    void StaticMesh::CallRemove()
    {
        bool successed = Renderer::RemoveMesh(this);
    }

    //////////////////////////////////////////////////////////
    //--------------------Dynamic Mesh----------------------//
    //////////////////////////////////////////////////////////
    DynamicMesh::DynamicMesh(Ref<MeshSource> meshSource)
        : Mesh(meshSource)
    {
        m_MeshSource->AfterGenerate(true);
        m_RenderSource = std::make_shared<RenderSource>(GetMaxCount<Vertex>(), GetMaxCount<Index>());
        m_RenderSource->VBO->SetData(0, m_MeshSource->GetVertexs().size() * sizeof(Vertex), m_MeshSource->GetVertexs().data());
        m_RenderSource->IBO->SetData(0, m_MeshSource->GetIndexs().size() * 3, m_MeshSource->GetIndexs().data());
    }

    //////////////////////////////////////////////////////////
    //--------------------Instance Mesh---------------------//
    //////////////////////////////////////////////////////////
    InstanceMesh::InstanceMesh(Ref<MeshSource> meshSource)
    {
    }
}

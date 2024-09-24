#include "Mesh.h"

#include "Renderer.h"

namespace Chozo
{

    //////////////////////////////////////////////////////////////////////////////////
	// MeshSource
	//////////////////////////////////////////////////////////////////////////////////

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

    //////////////////////////////////////////////////////////////////////////////////
	// DynamicMesh
	//////////////////////////////////////////////////////////////////////////////////

    DynamicMesh::DynamicMesh(Ref<MeshSource> meshSource)
        : Mesh(meshSource)
    {
        Init();
    }

    void DynamicMesh::Init()
    {
        m_MeshSource->AfterGenerate(true);
        m_RenderSource = Ref<RenderSource>::Create(GetMaxCount<Vertex>(), GetMaxCount<Index>());
        m_RenderSource->VBO->SetData(0, m_MeshSource->GetVertexs().size() * sizeof(Vertex), m_MeshSource->GetVertexs().data());
        m_RenderSource->IBO->SetData(0, m_MeshSource->GetIndexs().size() * 3, m_MeshSource->GetIndexs().data());
    }

    //////////////////////////////////////////////////////////////////////////////////
	// InstancedMesh
	//////////////////////////////////////////////////////////////////////////////////

    InstancedMesh::InstancedMesh(Ref<MeshSource> meshSource)
    {
    }

    //////////////////////////////////////////////////////////////////////////////////
	// StaticMesh
	//////////////////////////////////////////////////////////////////////////////////

    void StaticMesh::OnSubmit(bool successed)
    {
        m_MeshSource->AfterGenerate(successed);
    }

    void StaticMesh::CallRemove()
    {
        CZ_CORE_INFO("CallRemove");
        m_MeshSource->SetLocalTransform(glm::mat4(1.0f));
        Renderer::RemoveStaticMesh(this);
    }
}

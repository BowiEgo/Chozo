#include "Mesh.h"

#include "Renderer.h"
#include "Chozo/FileSystem/MeshImporter.h"

#include "Chozo/Core/Application.h"

namespace Chozo
{

    //////////////////////////////////////////////////////////////////////////////////
	// MeshSource
	//////////////////////////////////////////////////////////////////////////////////
    MeshSource::MeshSource(const std::string& path)
    {
    }

    MeshSource::MeshSource(const std::vector<Vertex> &vertexs, const std::vector<Index> &indexs, const uint32_t &indexCount, const uint32_t &indicesCount)
    {
        m_Buffer.Vertexs = vertexs;
        m_Buffer.Indexs = indexs;
        m_Buffer.IndexCount = indexCount;
        m_Buffer.IndicesCount = indicesCount;
    }

    Ref<MeshSource> MeshSource::Create(const std::string &path)
    {
        return MeshImporter::ToMeshSourceFromFile(path);
    }

    Mesh::Mesh()
    {
    }

    void Mesh::Invalidate()
    {
        if (!m_RenderSource)
            m_RenderSource = Ref<RenderSource>::Create(GetMaxCount<Vertex>(), GetMaxCount<Index>());
        
        m_RenderSource->VBO->SetData(0, m_MeshSource->GetVertexs().size() * sizeof(Vertex), m_MeshSource->GetVertexs().data());
        m_RenderSource->IBO->SetData(0, m_MeshSource->GetIndexs().size() * 3, m_MeshSource->GetIndexs().data());

		const auto& meshMaterials = m_MeshSource->GetMaterials();
        m_Materials = Ref<MaterialTable>::Create((uint32_t)meshMaterials.size());
        for (size_t i = 0; i < meshMaterials.size(); i++)
            m_Materials->SetMaterial((uint32_t)i, meshMaterials[i]);
    }

    //////////////////////////////////////////////////////////////////////////////////
	// DynamicMesh
	//////////////////////////////////////////////////////////////////////////////////
    DynamicMesh::DynamicMesh(Ref<MeshSource> meshSource)
        : Mesh(meshSource)
    {
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
        // m_MeshSource->AfterGenerate(successed);
    }

    void StaticMesh::CallRemove()
    {
        // CZ_CORE_INFO("CallRemove");
        // m_MeshSource->SetLocalTransform(glm::mat4(1.0f));
        // Renderer::RemoveStaticMesh(this);
    }
}

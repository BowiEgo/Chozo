#include <Core/Memory/Memory.hpp>
#include <Runtime/RHI/RHIAPI.hpp>
#include <Runtime/RenderCore/Mesh.hpp>

namespace CZ {

const std::string Mesh::GetName() const { return m_Obj->GetName(); }

template <> void Handle<MeshObj>::Destroy() {
    if (m_Obj) {
        m_Obj->VertexBuffer.Destroy();
        m_Obj->IndexBuffer.Destroy();

        Delete(m_Obj);
        m_Obj = nullptr;
    }
}

void MeshObj::Upload() {
    if (!MeshBuffer.IsValid()) {
        CZ_RENDERCORE_LOG(Error, "No vertex or index data to upload");
        return;
    }

    {
        GraphicsBufferSpecification spec;
        spec.Size       = MeshBuffer.GetVertexBufferSize();
        spec.Usage      = BufferUsage::VertexBuffer;
        spec.MemoryType = MemoryType;
        spec.Name       = "Mesh_VertexBuffer";

        SafeBuffer data = SafeBuffer::Copy(MeshBuffer.Vertices.data(), spec.Size);

        VertexBuffer = RHIAPI::Get()->CreateGraphicsBuffer(spec, &data);
        if (!VertexBuffer) {
            CZ_RENDERCORE_LOG(Error, "Failed to create vertex buffer");
            return;
        }
    }

    {
        std::vector<uint32> flatIndices;
        flatIndices.reserve(MeshBuffer.GetIndexCount());
        for (const auto& idx : MeshBuffer.Indices) {
            flatIndices.push_back(idx.V1);
            flatIndices.push_back(idx.V2);
            flatIndices.push_back(idx.V3);
        }

        GraphicsBufferSpecification spec;
        spec.Size       = flatIndices.size() * sizeof(uint32);
        spec.Usage      = BufferUsage::IndexBuffer;
        spec.MemoryType = MemoryType;
        spec.Name       = "Mesh_IndexBuffer";

        SafeBuffer data = SafeBuffer::Copy(flatIndices.data(), spec.Size);

        IndexBuffer = RHIAPI::Get()->CreateGraphicsBuffer(spec, &data);
        if (!IndexBuffer) {
            CZ_RENDERCORE_LOG(Error, "Failed to create index buffer");
            return;
        }
    }

    CZ_RENDERCORE_LOG(Info, "Uploaded mesh with {} vertices, {} indices",
                      MeshBuffer.GetVertexCount(), MeshBuffer.GetIndexCount());
}

} // namespace CZ

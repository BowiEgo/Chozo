#include <string>
#include <vector>

#include <Core/Memory/Memory.hpp>
#include <Runtime/RHI/RHIAPI.hpp>
#include <Runtime/RenderCore/Mesh.hpp>

namespace CZ {

const std::string Mesh::GetName() const { return m_Obj->GetName(); }

// The mesh owns its GPU buffers; they are released by the Scope members.

void MeshObj::Upload() {
    if (!MeshBuffer.IsValid()) {
        CZ_RENDERCORE_LOG(Error, "No vertex or index data to upload");
        return;
    }

    if (m_VertexBuffer || m_IndexBuffer) {
        // Re-uploading: release the buffers created by the previous upload instead of
        // orphaning them. They may still be referenced by in-flight frames, so idle the
        // device first (same policy as Viewport::Resize). A frame-deferred deletion
        // queue in the RHI would remove the need for this stall.
        RHIAPI::Get()->WaitIdle();

        m_VertexBuffer.reset();
        m_IndexBuffer.reset();
    }

    {
        GraphicsBufferSpecification spec;
        spec.Size       = MeshBuffer.GetVertexBufferSize();
        spec.Usage      = BufferUsage::VertexBuffer;
        spec.MemoryType = MemoryType;
        spec.Name       = "Mesh_VertexBuffer";

        SafeBuffer data = SafeBuffer::Copy(MeshBuffer.Vertices.data(), spec.Size);

        m_VertexBuffer = RHIAPI::Get()->CreateGraphicsBuffer(spec, &data);
        if (!m_VertexBuffer) {
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

        m_IndexBuffer = RHIAPI::Get()->CreateGraphicsBuffer(spec, &data);
        if (!m_IndexBuffer) {
            CZ_RENDERCORE_LOG(Error, "Failed to create index buffer");
            return;
        }
    }

    CZ_RENDERCORE_LOG(Info, "Uploaded mesh with {} vertices, {} indices",
                      MeshBuffer.GetVertexCount(), MeshBuffer.GetIndexCount());
}

} // namespace CZ

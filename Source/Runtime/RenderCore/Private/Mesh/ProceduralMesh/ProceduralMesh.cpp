#include "ProceduralMesh.h"

#include "RHIAPI.h"

DEFINE_LOG_CATEGORY(LogProceduralMesh);

FProceduralMesh::~FProceduralMesh() {}

void FProceduralMesh::Upload() {
    if (!m_Buffer.IsValid()) {
        CZ_LOG(LogProceduralMesh, Error, "No vertex or index data to upload");
        return;
    }

    {
        FBufferSpecification spec;
        spec.Size       = m_Buffer.GetVertexBufferSize();
        spec.Usage      = EBufferUsage::VertexBuffer;
        spec.MemoryType = EMemoryType::HostVisible | EMemoryType::HostCoherent;
        spec.Name       = "ProceduralMesh_VertexBuffer";

        FBuffer data(m_Buffer.Vertices.data(), spec.Size);

        m_VertexBuffer = IRHIAPI::CreateBuffer(spec, data);
        if (!m_VertexBuffer) {
            CZ_LOG(LogProceduralMesh, Error, "Failed to create vertex buffer");
            return;
        }
    }

    {
        std::vector<uint32> flatIndices;
        flatIndices.reserve(m_Buffer.GetIndexCount());
        for (const auto& idx : m_Buffer.Indices) {
            flatIndices.push_back(idx.V1);
            flatIndices.push_back(idx.V2);
            flatIndices.push_back(idx.V3);
        }

        FBufferSpecification spec;
        spec.Size       = flatIndices.size() * sizeof(uint32);
        spec.Usage      = EBufferUsage::IndexBuffer;
        spec.MemoryType = EMemoryType::HostVisible | EMemoryType::HostCoherent;
        spec.Name       = "ProceduralMesh_IndexBuffer";

        FBuffer data(flatIndices.data(), spec.Size);

        m_IndexBuffer = IRHIAPI::CreateBuffer(spec, data);
        if (!m_IndexBuffer) {
            CZ_LOG(LogProceduralMesh, Error, "Failed to create index buffer");
            return;
        }
    }

    CZ_LOG(LogProceduralMesh, Info, "Uploaded mesh with {} vertices, {} indices",
           m_Buffer.GetVertexCount(), m_Buffer.GetIndexCount());
}

void FProceduralMesh::Draw(IRHICommandList* cmdList) const {
    if (!m_VertexBuffer || !m_IndexBuffer) {
        CZ_LOG(LogProceduralMesh, Error, "Buffers not uploaded");
        return;
    }

    cmdList->BindVertexBuffer(m_VertexBuffer, 0);
    cmdList->BindIndexBuffer(m_IndexBuffer);
    cmdList->DrawIndexed(m_Buffer.GetIndexCount());
}

// void FProceduralMesh::Backup() {
//     CZ_LOG(LogProceduralMesh, Trace, "Backup called - no implementation");
// }

// void FProceduralMesh::Backtrace() {
//     CZ_LOG(LogProceduralMesh, Trace, "Backtrace called - no implementation");
// }
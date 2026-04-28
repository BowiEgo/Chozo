#pragma once

#include "Asset.h"
#include "MeshTypes.h"
#include "Params.h"
#include "RHIBuffer.h"
#include "RHIContext.h"

#include "RenderCoreExport.h"

DECLARE_LOG_CATEGORY_EXTERN(LogProceduralMesh, Info);

class RENDER_CORE_API FProceduralMesh : public IAsset {
public:
    FProceduralMesh() = default;
    ~FProceduralMesh();

    virtual const std::string GetName() const override { return "ProceduralMesh"; };
    virtual const EAssetType GetType() const override { return EAssetType::Mesh; }

    virtual FMeshBuffer* GenerateBuffer()         = 0;
    virtual void SetParams(const IParams& params) = 0;

    void Upload();

    TRef<IRHIBuffer> GetVertexBuffer() const { return m_VertexBuffer; }
    TRef<IRHIBuffer> GetIndexBuffer() const { return m_IndexBuffer; }
    uint32 GetIndexCount() const { return m_Buffer.GetIndexCount(); }

    void Draw(IRHICommandList* cmdList) const;

protected:
    FMeshBuffer m_Buffer;
    FMatrix4 m_LocalTransform{ 1.0f };

private:
    TRef<IRHIBuffer> m_VertexBuffer;
    TRef<IRHIBuffer> m_IndexBuffer;
};
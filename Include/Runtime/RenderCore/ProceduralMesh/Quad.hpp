#pragma once

#include "ProceduralMesh.h"

class RENDER_CORE_API FQuad : public FProceduralMesh {
public:
    FQuad();
    virtual ~FQuad() = default;

    virtual FMeshBuffer* GenerateBuffer() override;

    virtual void SetParams(const IParams& params) override {};
};
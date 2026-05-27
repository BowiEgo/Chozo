#pragma once

#include <Runtime/RenderCore/Asset.hpp>
#include <Runtime/RenderCore/Mesh.hpp>
#include <Runtime/RenderCore/Params.hpp>

namespace CZ {

class ProceduralMesh : public Mesh {
public:
    ProceduralMesh() { m_Obj->MemoryType = MemoryType::HostVisible | MemoryType::HostCoherent; }
    virtual ~ProceduralMesh() = default;

    virtual const std::string GetName() const override { return "ProceduralMesh"; }

    virtual MeshBuffer* GenerateBuffer()         = 0;
    virtual void SetParams(const Params& params) = 0;
};

} // namespace CZ
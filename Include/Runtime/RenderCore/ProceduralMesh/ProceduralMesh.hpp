#pragma once

#include <Runtime/RenderCore/Asset.hpp>
#include <Runtime/RenderCore/Mesh.hpp>
#include <Runtime/RenderCore/MeshParams.hpp>

namespace CZ {

class ProceduralMesh : public Mesh {
public:
    explicit ProceduralMesh(MeshObj* obj) : Mesh(obj) {
        m_Obj->MemoryType = MemoryType::HostVisible | MemoryType::HostCoherent;
    }
    virtual ~ProceduralMesh() { m_Params.Destroy(); }

    virtual const std::string GetName() const override { return "ProceduralMesh"; }

    virtual MeshBuffer* GenerateBuffer()            = 0;
    virtual void SetParams(const MeshParams params) = 0;

protected:
    MeshParams m_Params;
};

} // namespace CZ
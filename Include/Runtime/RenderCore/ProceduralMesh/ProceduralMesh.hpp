#pragma once

#include <Core/TypeRegistry/TypeRegistry.hpp>
#include <Runtime/RenderCore/Asset.hpp>
#include <Runtime/RenderCore/Mesh.hpp>
#include <Runtime/RenderCore/MeshParams.hpp>

namespace CZ {

struct MeshGenerator {
    virtual ~MeshGenerator()                                                  = default;
    virtual MeshBuffer* GenerateBuffer(MeshParams params, MeshBuffer* buffer) = 0;
};

class ProceduralMesh : public Mesh {
public:
    explicit ProceduralMesh(MeshObj* obj) : Mesh(obj) {
        m_Obj->MemoryType = MemoryType::HostVisible | MemoryType::HostCoherent;
    }
    ~ProceduralMesh() { m_Params.Destroy(); }

    const std::string GetName() const override { return "ProceduralMesh"; }

    void SetParams(const MeshParams params) { m_Params = params.Clone(); }

    const std::string GetTypeName() const { return m_Params->GetTypeName(); }

    MeshBuffer* GenerateBuffer();

    static void RegisterType(const std::string& typeName, Scope<MeshGenerator> generator) {
        TypeRegister::Get().RegisterType("ProceduralMesh_" + typeName, true, TypeCategory::Mesh);
        s_Generators[typeName] = std::move(generator);
    }

    static void Shutdown() {
        for (auto& [name, generator] : s_Generators) {
            generator.reset();
        }
    }

protected:
    MeshParams m_Params;
    static std::unordered_map<std::string, Scope<MeshGenerator>> s_Generators;
};

} // namespace CZ
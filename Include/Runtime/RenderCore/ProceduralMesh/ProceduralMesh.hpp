#pragma once
#include <string>
#include <unordered_map>
#include <utility>

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

    const std::string GetName() const override { return "ProceduralMesh"; }

    const std::string GetTypeName() const {
        MeshParams params = m_Obj->GetParams();
        return params ? params->GetTypeName() : std::string();
    }

    /// Regenerates the CPU mesh buffer from the parameters owned by the mesh object.
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
    static std::unordered_map<std::string, Scope<MeshGenerator>> s_Generators;
};

} // namespace CZ
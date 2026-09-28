#include <string>
#include <unordered_map>

#include <Core/Log/LogMacros.hpp>
#include <Runtime/RenderCore/ProceduralMesh/ProceduralMesh.hpp>

namespace CZ {

std::unordered_map<std::string, Scope<MeshGenerator>> ProceduralMesh::s_Generators;

MeshBuffer* ProceduralMesh::GenerateBuffer() {
    MeshParams params = (*this)->GetParams();
    if (!params) {
        CZ_RENDERCORE_LOG(Error, "Cannot generate a mesh without parameters.");
        return nullptr;
    }

    const std::string typeName = params->GetTypeName();
    auto generator             = s_Generators.find(typeName);
    if (generator == s_Generators.end() || !generator->second) {
        CZ_RENDERCORE_LOG(Error, "No mesh generator registered for type '{}'.", typeName);
        return nullptr;
    }

    (*this)->MeshBuffer.Clear();
    generator->second->GenerateBuffer(params, &(*this)->MeshBuffer);

    return &(*this)->MeshBuffer;
}

} // namespace CZ

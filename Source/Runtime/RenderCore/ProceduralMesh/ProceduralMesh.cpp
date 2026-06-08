#include <Runtime/RenderCore/ProceduralMesh/ProceduralMesh.hpp>

namespace CZ {

std::unordered_map<std::string, Scope<MeshGenerator>> ProceduralMesh::s_Generators;

MeshBuffer* ProceduralMesh::GenerateBuffer() {
    (*this)->MeshBuffer.Clear();

    s_Generators[GetTypeName()]->GenerateBuffer(m_Params, &(*this)->MeshBuffer);

    return &(*this)->MeshBuffer;
}

} // namespace CZ
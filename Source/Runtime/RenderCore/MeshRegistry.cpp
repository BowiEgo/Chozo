#include "Core/TypeRegistry/TypeRegistry.hpp"
#include <Runtime/RenderCore/Mesh.hpp>
#include <Runtime/RenderCore/MeshRegistry.hpp>

namespace CZ {

Scope<MeshObj> ResourceLoaderTraits<MeshObj>::Load(const std::string& virtualPath) {
    auto realPath = VFS::Resolve(virtualPath);
    auto name     = realPath.stem();

    return CZ_CREATE_SCOPE(MEMORY_USAGE_ASSET, MeshObj);
}

Scope<MeshObj> Create(const MeshParams& params) {
    (void)params;
    return nullptr;
}

template <> void AssetRegistry<MeshObj>::Init() {
    TypeRegister::Get().RegisterType("ProceduralMesh_Cube", true, TypeCategory::Mesh);
    TypeRegister::Get().RegisterType("ProceduralMesh_Sphere", true, TypeCategory::Mesh);
}

} // namespace CZ
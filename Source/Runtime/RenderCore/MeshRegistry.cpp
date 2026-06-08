#include <Runtime/RenderCore/Mesh.hpp>
#include <Runtime/RenderCore/MeshRegistry.hpp>
#include <Runtime/RenderCore/ProceduralMesh/ProceduralMesh.hpp>

#include "./ProceduralMesh/CubeGenerator.hpp"

namespace CZ {

Scope<MeshObj> ResourceLoaderTraits<MeshObj>::Load(const std::string& virtualPath) {
    auto realPath = VFS::Resolve(virtualPath);
    auto name     = realPath.stem();

    return CZ_CREATE_SCOPE(MEMORY_USAGE_ASSET, MeshObj);
}

Scope<MeshObj> ResourceGeneratorTraits<MeshObj>::Generate(const MeshParams params) {
    auto* meshObj = CZ_NEW(MEMORY_USAGE_ASSET, MeshObj);
    auto mesh     = ProceduralMesh(meshObj);
    mesh.SetParams(params);
    mesh.GenerateBuffer();

    return Scope<MeshObj>(meshObj);
}

template <> void AssetRegistry<MeshObj>::Init() {
    // TypeRegister::Get().RegisterType("ProceduralMesh_Sphere", true, TypeCategory::Mesh);

    ProceduralMesh::RegisterType("Cube", CZ_CREATE_SCOPE(MEMORY_USAGE_RUNTIME, CubeGenerator));
}

template <> void AssetRegistry<MeshObj>::Shutdown() {
    Clear();
    ProceduralMesh::Shutdown();
}

} // namespace CZ
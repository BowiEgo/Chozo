#include <string>

#include <Runtime/RenderCore/Mesh.hpp>
#include <Runtime/RenderCore/MeshRegistry.hpp>
#include <Runtime/RenderCore/ProceduralMesh/ProceduralMesh.hpp>

#include "./ProceduralMesh/CubeGenerator.hpp"
#include "./ProceduralMesh/CylinderGenerator.hpp"
#include "./ProceduralMesh/PlaneGenerator.hpp"
#include "./ProceduralMesh/SphereGenerator.hpp"
#include "./ProceduralMesh/TorusGenerator.hpp"

namespace CZ {

Scope<MeshObj> ResourceLoaderTraits<MeshObj>::Load(const std::string& virtualPath) {
    auto realPath = VFS::Resolve(virtualPath);
    auto name     = realPath.stem();

    return CZ_CREATE_SCOPE(MEMORY_USAGE_ASSET, MeshObj);
}

Scope<MeshObj> ResourceGeneratorTraits<MeshObj>::Generate(const MeshParams params) {
    auto meshObj = CZ_CREATE_SCOPE(MEMORY_USAGE_ASSET, MeshObj);

    // The mesh owns its parameters from here on; generation reads them back from the object.
    meshObj->SetParams(params);
    ProceduralMesh(meshObj.get()).GenerateBuffer();

    return meshObj;
}

template <> void AssetRegistry<MeshObj>::Init() {

    ProceduralMesh::RegisterType("Cube", CZ_CREATE_SCOPE(MEMORY_USAGE_RUNTIME, CubeGenerator));
    ProceduralMesh::RegisterType("Sphere", CZ_CREATE_SCOPE(MEMORY_USAGE_RUNTIME, SphereGenerator));
    ProceduralMesh::RegisterType("Plane", CZ_CREATE_SCOPE(MEMORY_USAGE_RUNTIME, PlaneGenerator));
    ProceduralMesh::RegisterType("Cylinder",
                                 CZ_CREATE_SCOPE(MEMORY_USAGE_RUNTIME, CylinderGenerator));
    ProceduralMesh::RegisterType("Torus", CZ_CREATE_SCOPE(MEMORY_USAGE_RUNTIME, TorusGenerator));
}

template <> void AssetRegistry<MeshObj>::Shutdown() {
    Clear();
    ProceduralMesh::Shutdown();
}

} // namespace CZ
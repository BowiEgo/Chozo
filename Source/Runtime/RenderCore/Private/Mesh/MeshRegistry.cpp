#include "MeshRegistry.h"

#include "Cube.h"
#include "Sphere.h"

FMeshRegistry& FMeshRegistry::Get() {
    static FMeshRegistry instance;
    return instance;
}

namespace {

struct RegisterBuiltinMeshes {
    RegisterBuiltinMeshes() {
        auto& registry = FMeshRegistry::Get();

        registry.RegisterMeshType<FCubeParams, FCube>("Cube", true);
        registry.RegisterMeshType<FSphereParams, FSphere>("Sphere", true);
        // CZ_LOG(LogMesh, Info, "Built-in mesh types registered");
    }
} s_RegisterBuiltinMeshes;

} // namespace

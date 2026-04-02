#include "MeshRegister.h"

#include "Cube.h"
#include "Sphere.h"

FMeshRegister& FMeshRegister::Get() {
    static FMeshRegister instance;
    return instance;
}

void FMeshRegister::Init() {
    auto& registry = FMeshRegister::Get();
    registry.RegisterMeshType<FCubeParams, FCube>("Cube", true);
    registry.RegisterMeshType<FSphereParams, FSphere>("Sphere", true);
    // CZ_LOG(LogMesh, Info, "Built-in mesh types registered");
}

// namespace {

// struct RegisterBuiltinMeshes {
//     RegisterBuiltinMeshes() {
//         auto& registry = FMeshRegister::Get();

//         registry.RegisterMeshType<FCubeParams, FCube>("Cube", true);
//         registry.RegisterMeshType<FSphereParams, FSphere>("Sphere", true);
//         // CZ_LOG(LogMesh, Info, "Built-in mesh types registered");
//     }
// } s_RegisterBuiltinMeshes;

// } // namespace

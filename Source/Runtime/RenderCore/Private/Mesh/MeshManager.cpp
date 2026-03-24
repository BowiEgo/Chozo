#include "MeshManager.h"

FMeshManager& FMeshManager::Get() {
    static FMeshManager instance;
    return instance;
}
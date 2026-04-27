#include "MeshManager.h"

CMeshManager& CMeshManager::Get() {
    static CMeshManager instance;
    return instance;
}
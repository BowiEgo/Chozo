#pragma once

#include "MeshParams.h"
#include "MeshRegistry.h"
#include "ProceduralMesh.h"
#include "Ref.h"
#include "RegistryManager.h"
#include "Scope.h"

#include "RenderCoreExport.h"

DECLARE_LOG_CATEGORY_EXTERN(LogMeshManager, Info);

class RENDER_CORE_API FMeshManager {
public:
    static FMeshManager& Get();

    FMeshManager& operator=(const FMeshManager&) = delete;

    TRef<FProceduralMesh> CreateProceduralMesh(const IParams& params) {
        TRef<FProceduralMesh> mesh = FMeshRegistry::Get().CreateMesh(params);
        FAssetHandle handle = FAssetHandle::Generate();

        mesh->SetHandle(handle);
        m_ProceduralMeshes[handle] = mesh;

        return mesh;
    }

    TRef<FProceduralMesh> GetMesh(FAssetHandle handle) {
        auto it = m_ProceduralMeshes.find(handle);
        if (it != m_ProceduralMeshes.end()) {
            return it->second;
        }

        return nullptr;
    }

    void UpdateMesh(const FAssetHandle handle, const IParams& params) {
        auto mesh = GetMesh(handle);
        if (!mesh) return;

        mesh->SetParams(params);
        mesh->GenerateBuffer();
    }

    void Shutdown() { m_ProceduralMeshes.clear(); }

private:
    std::unordered_map<FAssetHandle, TRef<FProceduralMesh>> m_ProceduralMeshes;
};

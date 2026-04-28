#pragma once

#include "MeshParamsWrapper.h"
#include "MeshRegister.h"
#include "ProceduralMesh.h"
#include "Ref.h"
#include "Scope.h"
#include "TypeRegister.h"

#include "RenderCoreExport.h"

DECLARE_LOG_CATEGORY_EXTERN(LogMeshManager, Info);

class RENDER_CORE_API CMeshManager {
public:
    static CMeshManager& Get();

    CMeshManager& operator=(const CMeshManager&) = delete;

    TRef<FProceduralMesh> CreateProceduralMesh(const IParams& params) {
        TRef<FProceduralMesh> mesh = FMeshRegister::Get().CreateMesh(params);
        FAssetHandle handle        = FAssetHandle::Generate();

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

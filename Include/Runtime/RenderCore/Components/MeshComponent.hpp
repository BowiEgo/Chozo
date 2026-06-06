#pragma once

#include <Runtime/RenderCore/MeshParams.hpp>
#include <Runtime/RenderCore/MeshRegistry.hpp>

namespace CZ {

struct MeshComponent {
    // ===== Core Data =====
    MeshParams MeshParams;
    AssetHandle MeshHandle = AssetHandle::Invalid();

    // ===== State =====
    mutable uint32_t Revision = 0;
    mutable bool bIsDirty     = true;

    // ===== State Management =====
    void MarkDirty() const {
        bIsDirty = true;
        Revision++;
    }

    void ClearDirty() const { bIsDirty = false; }
    bool IsDirty() const { return bIsDirty; }
    bool IsValid() const { return !!MeshParams; }

    // ===== Constructors =====
    MeshComponent() = default;
    explicit MeshComponent(MeshParams& params) : MeshParams(params) {
        MeshHandle = MeshManager::Get().CreateProceduralMesh(*params.Get())->GetHandle();
        MeshHandle = MeshRegistry::Create(params);
    }

    // ===== Type Helpers =====
    // EMeshType GetType() const { return MeshParams.GetType(); }
    std::string GetTypeName() const { return MeshParams.GetTypeName(); }

    void SetMeshParams(const MeshParams& props) {
        if (MeshParams == props) return;
        MeshParams = props.Clone();

        if (!MeshHandle.IsValid()) {
            MeshHandle = MeshManager::Get().CreateProceduralMesh(*props.Get())->GetHandle();
        }

        MarkDirty();
    }

    void UpdateMesh() {
        if (bIsDirty) {
            if (MeshHandle.IsValid())
                MeshManager::Get().UpdateMesh(MeshHandle, *MeshParams.Get());
            else
                MeshHandle =
                    MeshManager::Get().CreateProceduralMesh(*MeshParams.Get())->GetHandle();

            ClearDirty();
        }
    }

    // ===== Comparison =====
    bool operator==(const MeshComponent& other) const { return MeshParams == other.MeshParams; }
    bool operator!=(const MeshComponent& other) const { return !(*this == other); }

    // ===== Hash =====
    size_t GetHash() const { return MeshParams.GetHash(); }
};

} // namespace CZ

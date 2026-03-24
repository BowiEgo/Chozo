#pragma once

#include "CoreMinimal.h"
#include "MeshManager.h"
#include "MeshParams.h"
#include "MeshReflection.h"
#include "MeshRegistry.h"

struct FMeshComponent {
    // ===== Core Data =====
    FMeshParams MeshParams;
    FAssetHandle MeshHandle = FAssetHandle::Invalid();

    // ===== State =====
    mutable uint32_t Revision = 0;
    mutable bool bIsDirty = true;

    // ===== State Management =====
    void MarkDirty() const {
        bIsDirty = true;
        Revision++;
    }

    void ClearDirty() const { bIsDirty = false; }
    bool IsDirty() const { return bIsDirty; }
    bool IsValid() const { return MeshParams.Get() != nullptr; }

    // ===== Constructors =====
    FMeshComponent() = default;
    explicit FMeshComponent(FMeshParams& params) : MeshParams(params) {
        MeshHandle = FMeshManager::Get().CreateProceduralMesh(*params.Get())->GetHandle();
    }

    // ===== Type Helpers =====
    // EMeshType GetType() const { return MeshParams.GetType(); }
    std::string GetTypeName() const { return MeshParams.GetTypeName(); }

    // ===== Generic Property Setters =====
    template <typename Tag> void SetProperty(typename MeshPropertyTraits<Tag>::Type value) {
        bool set = false;

        std::visit(
            [&](auto& params) {
                using T = std::decay_t<decltype(params)>;
                set = SetProperty(params, Tag{}, value);
            },
            MeshParams);

        if (set) {
            MarkDirty();
        }
    }

    // ===== Generic Property Getters =====
    template <typename Tag>
    std::optional<typename MeshPropertyTraits<Tag>::Type> GetProperty() const {
        std::optional<typename MeshPropertyTraits<Tag>::Type> result;

        std::visit(
            [&](const auto& params) {
                using T = std::decay_t<decltype(params)>;
                result = GetProperty(params, Tag{});
            },
            MeshParams);

        return result;
    }

    void SetMeshParams(const FMeshParams& params) {
        if (MeshParams == params) return;
        MeshParams = params.Clone();

        if (!MeshHandle.IsValid()) {
            MeshHandle = FMeshManager::Get().CreateProceduralMesh(*params.Get())->GetHandle();
        }

        MarkDirty();
    }

    void UpdateMesh() {
        if (bIsDirty) {
            if (MeshHandle.IsValid())
                FMeshManager::Get().UpdateMesh(MeshHandle, *MeshParams.Get());
            else
                MeshHandle =
                    FMeshManager::Get().CreateProceduralMesh(*MeshParams.Get())->GetHandle();

            ClearDirty();
        }
    }

    // ===== Comparison =====
    bool operator==(const FMeshComponent& other) const { return MeshParams == other.MeshParams; }
    bool operator!=(const FMeshComponent& other) const { return !(*this == other); }

    // ===== Hash =====
    size_t GetHash() const { return MeshParams.GetHash(); }
};

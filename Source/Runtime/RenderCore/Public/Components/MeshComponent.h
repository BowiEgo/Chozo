#pragma once

#include "CoreMinimal.h"
#include "MeshManager.h"
#include "MeshParamsWrapper.h"
#include "MeshReflection.h"
#include "MeshRegister.h"

struct FMeshComponent {
    // ===== Core Data =====
    FMeshParamsWrapper MeshParamsWrapper;
    FAssetHandle MeshHandle = FAssetHandle::Invalid();

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
    bool IsValid() const { return MeshParamsWrapper.Get() != nullptr; }

    // ===== Constructors =====
    FMeshComponent() = default;
    explicit FMeshComponent(FMeshParamsWrapper& props) : MeshParamsWrapper(props) {
        MeshHandle = CMeshManager::Get().CreateProceduralMesh(*props.Get())->GetHandle();
    }

    // ===== Type Helpers =====
    // EMeshType GetType() const { return MeshParamsWrapper.GetType(); }
    std::string GetTypeName() const { return MeshParamsWrapper.GetTypeName(); }

    // ===== Generic Property Setters =====
    template <typename Tag> void SetProperty(typename MeshPropertyTraits<Tag>::Type value) {
        bool set = false;

        std::visit(
            [&](auto& props) {
                using T = std::decay_t<decltype(props)>;
                set     = SetProperty(props, Tag{}, value);
            },
            MeshParamsWrapper);

        if (set) {
            MarkDirty();
        }
    }

    // ===== Generic Property Getters =====
    template <typename Tag>
    std::optional<typename MeshPropertyTraits<Tag>::Type> GetParamValue() const {
        std::optional<typename MeshPropertyTraits<Tag>::Type> result;

        std::visit(
            [&](const auto& props) {
                using T = std::decay_t<decltype(props)>;
                result  = GetParamValue(props, Tag{});
            },
            MeshParamsWrapper);

        return result;
    }

    void SetMeshParamsWrapper(const FMeshParamsWrapper& props) {
        if (MeshParamsWrapper == props) return;
        MeshParamsWrapper = props.Clone();

        if (!MeshHandle.IsValid()) {
            MeshHandle = CMeshManager::Get().CreateProceduralMesh(*props.Get())->GetHandle();
        }

        MarkDirty();
    }

    void UpdateMesh() {
        if (bIsDirty) {
            if (MeshHandle.IsValid())
                CMeshManager::Get().UpdateMesh(MeshHandle, *MeshParamsWrapper.Get());
            else
                MeshHandle =
                    CMeshManager::Get().CreateProceduralMesh(*MeshParamsWrapper.Get())->GetHandle();

            ClearDirty();
        }
    }

    // ===== Comparison =====
    bool operator==(const FMeshComponent& other) const {
        return MeshParamsWrapper == other.MeshParamsWrapper;
    }
    bool operator!=(const FMeshComponent& other) const { return !(*this == other); }

    // ===== Hash =====
    size_t GetHash() const { return MeshParamsWrapper.GetHash(); }
};

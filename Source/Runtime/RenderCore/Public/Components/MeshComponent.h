#pragma once

#include "CoreMinimal.h"
#include "MeshManager.h"
#include "MeshProps.h"
#include "MeshReflection.h"
#include "MeshRegister.h"

struct FMeshComponent {
    // ===== Core Data =====
    FMeshProps MeshProps;
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
    bool IsValid() const { return MeshProps.Get() != nullptr; }

    // ===== Constructors =====
    FMeshComponent() = default;
    explicit FMeshComponent(FMeshProps& props) : MeshProps(props) {
        MeshHandle = CMeshManager::Get().CreateProceduralMesh(*props.Get())->GetHandle();
    }

    // ===== Type Helpers =====
    // EMeshType GetType() const { return MeshProps.GetType(); }
    std::string GetTypeName() const { return MeshProps.GetTypeName(); }

    // ===== Generic Property Setters =====
    template <typename Tag> void SetProperty(typename MeshPropertyTraits<Tag>::Type value) {
        bool set = false;

        std::visit(
            [&](auto& props) {
                using T = std::decay_t<decltype(props)>;
                set     = SetProperty(props, Tag{}, value);
            },
            MeshProps);

        if (set) {
            MarkDirty();
        }
    }

    // ===== Generic Property Getters =====
    template <typename Tag>
    std::optional<typename MeshPropertyTraits<Tag>::Type> GetProperty() const {
        std::optional<typename MeshPropertyTraits<Tag>::Type> result;

        std::visit(
            [&](const auto& props) {
                using T = std::decay_t<decltype(props)>;
                result  = GetProperty(props, Tag{});
            },
            MeshProps);

        return result;
    }

    void SetMeshProps(const FMeshProps& props) {
        if (MeshProps == props) return;
        MeshProps = props.Clone();

        if (!MeshHandle.IsValid()) {
            MeshHandle = CMeshManager::Get().CreateProceduralMesh(*props.Get())->GetHandle();
        }

        MarkDirty();
    }

    void UpdateMesh() {
        if (bIsDirty) {
            if (MeshHandle.IsValid())
                CMeshManager::Get().UpdateMesh(MeshHandle, *MeshProps.Get());
            else
                MeshHandle =
                    CMeshManager::Get().CreateProceduralMesh(*MeshProps.Get())->GetHandle();

            ClearDirty();
        }
    }

    // ===== Comparison =====
    bool operator==(const FMeshComponent& other) const { return MeshProps == other.MeshProps; }
    bool operator!=(const FMeshComponent& other) const { return !(*this == other); }

    // ===== Hash =====
    size_t GetHash() const { return MeshProps.GetHash(); }
};

#pragma once

#include <Runtime/App/Application.hpp>
#include <Runtime/RenderCore/MeshParams.hpp>
#include <Runtime/RenderCore/MeshRegistry.hpp>
#include <Runtime/RenderCore/ProceduralMesh/ProceduralMesh.hpp>

namespace CZ {

struct MeshComponent {
    // ===== Core Data =====
    MeshParams m_Params;
    AssetHandle m_Handle = AssetHandle::Invalid();

    // ===== State =====
    mutable uint32_t m_Revision = 0;
    mutable bool m_bIsDirty     = true;

    // ===== State Management =====
    void MarkDirty() const {
        m_bIsDirty = true;
        m_Revision++;
    }

    void ClearDirty() const { m_bIsDirty = false; }
    bool IsDirty() const { return m_bIsDirty; }
    bool IsValid() const { return !!m_Params; }

    // ===== Constructors =====
    MeshComponent() = default;
    explicit MeshComponent(MeshParams params) : m_Params(params) {
        m_Handle =
            Application::Get().GetEngine()->GetMeshRegistry()->GenerateAsset(params).GetHandle();
    }

    // ===== Type Helpers =====
    // EMeshType GetType() const { return MeshParams.GetType(); }
    std::string GetTypeName() const { return m_Params->GetTypeName(); }

    void SetMeshParams(const MeshParams params) {
        if (m_Params == params) return;
        m_Params = params.Clone();

        if (!m_Handle.IsValid()) {
            m_Handle = Application::Get()
                           .GetEngine()
                           ->GetMeshRegistry()
                           ->GenerateAsset(m_Params)
                           .GetHandle();
        }

        MarkDirty();
    }

    void UpdateMesh() {
        if (m_bIsDirty) {
            if (m_Handle.IsValid()) {
                auto meshObj =
                    Application::Get().GetEngine()->GetMeshRegistry()->GetAsset(m_Handle).Raw();

                auto mesh = ProceduralMesh(meshObj);
                mesh.SetParams(m_Params);
                mesh.GenerateBuffer();
            } else {
                m_Handle = Application::Get()
                               .GetEngine()
                               ->GetMeshRegistry()
                               ->GenerateAsset(m_Params)
                               .GetHandle();
            }

            ClearDirty();
        }
    }

    // ===== Comparison =====
    bool operator==(const MeshComponent& other) const { return m_Params == other.m_Params; }
    bool operator!=(const MeshComponent& other) const { return !(*this == other); }

    // ===== Hash =====
    size_t GetHash() const { return m_Params->GetHash(); }
};

} // namespace CZ

#pragma once
#include <cstdint>

#include <Runtime/RenderCore/MeshParams.hpp>
#include <Runtime/RenderCore/MeshRegistry.hpp>
#include <Runtime/RenderCore/ProceduralMesh/ProceduralMesh.hpp>

namespace CZ {

/**
 * Mesh component.
 *
 * The component owns nothing but an asset handle: the mesh object (owned by `MeshRegistry`)
 * owns the parameters it was generated from, so there is nothing to clone or free here.
 *
 * The registry is passed in by the caller (`SceneObj` holds the one it was configured with)
 * instead of being looked up through a global, which keeps the scene and its components
 * independent of the application singleton and therefore testable without a renderer.
 */
struct MeshComponent {
    // ===== Core Data =====
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
    bool IsValid() const { return m_Handle.IsValid(); }

    // ===== Constructors =====
    MeshComponent() = default;

    // ===== Params =====
    MeshParams GetMeshParams(MeshRegistry& registry) {
        Mesh mesh = GetMeshAsset(registry);
        return mesh ? mesh->GetParams() : MeshParams();
    }

    void SetMeshParams(const MeshParams params, MeshRegistry& registry) {
        if (!params) return;

        if (!m_Handle.IsValid()) {
            m_Handle = registry.GenerateAsset(params).GetHandle();
            MarkDirty();
            return;
        }

        Mesh mesh = registry.GetAsset(m_Handle);
        if (!mesh) return;

        MeshParams current = mesh->GetParams();
        if (current && *current.Get() == params.Get()) return;

        mesh->SetParams(params);
        MarkDirty();
    }

    /// Regenerates the CPU side mesh data for the current parameters.
    void UpdateMesh(MeshRegistry& registry) {
        if (!m_bIsDirty || !m_Handle.IsValid()) return;

        Mesh mesh = GetMeshAsset(registry);
        if (!mesh) return;

        ProceduralMesh(mesh.Raw()).GenerateBuffer();
        ClearDirty();
    }

    // ===== Comparison =====
    bool operator==(const MeshComponent& other) const { return m_Handle == other.m_Handle; }
    bool operator!=(const MeshComponent& other) const { return !(*this == other); }

private:
    Mesh GetMeshAsset(MeshRegistry& registry) {
        if (!m_Handle.IsValid()) return Mesh();
        return registry.GetAsset(m_Handle);
    }
};

} // namespace CZ

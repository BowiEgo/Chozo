#include <Runtime/RenderCore/Components/RelationshipComponent.hpp>
#include <Runtime/RenderCore/Components/TransformComponent.hpp>
#include <Runtime/RenderCore/Scene/Scene.hpp>
#include <Runtime/RenderCore/Scene/TransformSystem.hpp>

#include <Core/Log/LogMacros.hpp>

#include "SceneECS.hpp" // IWYU pragma: keep

namespace CZ {

/// Upper bound for hierarchy walks, guards against malformed (cyclic) hierarchies.
static constexpr uint32_t kMaxHierarchyDepth = 256;

void TransformSystem::Update() {
    if (m_DirtySet.empty()) return;

    // Get update order (sort by depth to ensure parent nodes are processed before children)
    auto updateOrder = GetUpdateOrder();

    for (Entity entity : updateOrder) {
        // The entity may have been destroyed after it was marked dirty.
        if (!m_SceneObj->IsValid(entity) || !m_SceneObj->HasComponent<TransformComponent>(entity)) {
            continue;
        }

        auto& transform = m_SceneObj->GetComponent<TransformComponent>(entity);
        if (!transform.IsValid() || !transform.IsDirty()) continue;

        UpdateEntity(entity);
        transform.ClearDirty();
    }

    m_DirtySet.clear();
}

std::vector<Entity> TransformSystem::GetUpdateOrder() {
    if (m_bNeedSort) {
        // Recalculate depth for all entities
        // 1. Find all root nodes (those with no parent)
        std::vector<Entity> roots;
        auto view = m_SceneObj->View<TransformComponent, RelationshipComponent>();
        for (auto enttHandle : view) {
            Entity entity = EntityFromEntt(enttHandle);
            auto& rel     = m_SceneObj->GetComponent<RelationshipComponent>(entity);
            if (!rel.HasParent()) {
                roots.push_back(entity);
            }
        }

        // 2. DFS to compute depth
        for (Entity root : roots) {
            ComputeDepth(root, 0);
        }
        m_bNeedSort = false;
    }

    // Sort by depth in ascending order (from shallow to deep). Entities that no longer
    // exist are dropped so the comparators below only touch live components.
    std::vector<Entity> sorted;
    sorted.reserve(m_DirtySet.size());

    for (Entity entity : m_DirtySet) {
        if (m_SceneObj->IsValid(entity) &&
            m_SceneObj->HasComponent<RelationshipComponent>(entity)) {
            sorted.push_back(entity);
        }
    }

    std::sort(sorted.begin(), sorted.end(), [this](Entity a, Entity b) {
        auto& ra = m_SceneObj->GetComponent<RelationshipComponent>(a);
        auto& rb = m_SceneObj->GetComponent<RelationshipComponent>(b);
        return ra.GetDepth() < rb.GetDepth();
    });
    return sorted;
}

void TransformSystem::ComputeDepth(Entity entity, uint32_t depth) {
    if (depth > kMaxHierarchyDepth) {
        CZ_LOG(LogScene, Error, "Transform hierarchy is too deep or contains a cycle.");
        return;
    }

    if (!m_SceneObj->IsValid(entity) || !m_SceneObj->HasComponent<RelationshipComponent>(entity)) {
        return;
    }

    auto& rel = m_SceneObj->GetComponent<RelationshipComponent>(entity);
    rel.SetDepth(depth);

    for (Entity child : rel.Children) {
        ComputeDepth(child, depth + 1);
    }
}

void TransformSystem::UpdateEntity(Entity entity, uint32_t recursionDepth) {
    if (recursionDepth > kMaxHierarchyDepth) {
        CZ_LOG(LogScene, Error, "Transform hierarchy is too deep or contains a cycle.");
        return;
    }

    if (!m_SceneObj->IsValid(entity) || !m_SceneObj->HasComponent<TransformComponent>(entity)) {
        return;
    }

    auto& transform = m_SceneObj->GetComponent<TransformComponent>(entity);
    if (!transform.IsValid()) return;

    Matrix4 local = transform.GetLocalMatrix();

    const RelationshipComponent* rel =
        m_SceneObj->HasComponent<RelationshipComponent>(entity)
            ? &m_SceneObj->GetComponent<RelationshipComponent>(entity)
            : nullptr;

    const bool hasLiveParent = rel && rel->HasParent() && m_SceneObj->IsValid(rel->Parent) &&
                               m_SceneObj->HasComponent<TransformComponent>(rel->Parent);

    if (hasLiveParent) {
        const Entity parent   = rel->Parent;
        auto& parentTransform = m_SceneObj->GetComponent<TransformComponent>(parent);

        if (parentTransform.IsValid() && !parentTransform.IsDirty()) {
            // Parent node already updated, use its world matrix
            transform.WorldMatrix = parentTransform.WorldMatrix * local;
        } else {
            // Parent node not yet updated (theoretically shouldn’t happen because sorting
            // guarantees parents are processed first). Update it recursively instead.
            UpdateEntity(parent, recursionDepth + 1);
            auto& updatedParent   = m_SceneObj->GetComponent<TransformComponent>(parent);
            transform.WorldMatrix = updatedParent.WorldMatrix * local;
        }
    } else {
        transform.WorldMatrix = local;
    }

    // Inverse transpose of the 3x3 part so normals survive non-uniform scaling.
    transform.WorldNormalMatrix = transform.WorldMatrix.ToMatrix3().Inverse().Transpose();
}

void TransformSystem::MarkDirty(Entity entity) {
    auto& transform = m_SceneObj->GetComponent<TransformComponent>(entity);
    // if (transform.IsDirty()) return;

    transform.MarkDirty();
    m_DirtySet.push_back(entity);
    m_bNeedSort = true; // Depth may change, need to reorder before next update

    // Recursively mark all child nodes (optional, because children will detect dirty parent during
    // update and recompute)
    auto& rel = m_SceneObj->GetComponent<RelationshipComponent>(entity);
    for (Entity child : rel.Children) {
        MarkDirty(child);
    }
}

} // namespace CZ
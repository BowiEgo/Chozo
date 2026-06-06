#include <Runtime/RenderCore/Components/RelationshipComponent.hpp>
#include <Runtime/RenderCore/Components/TransformComponent.hpp>
#include <Runtime/RenderCore/Scene/Scene.hpp>
#include <Runtime/RenderCore/Scene/TransformSystem.hpp>

#include "SceneECS.hpp" // IWYU pragma: keep

namespace CZ {

void TransformSystem::Update() {
    if (m_DirtySet.empty()) return;

    // Get update order (sort by depth to ensure parent nodes are processed before children)
    auto updateOrder = GetUpdateOrder();

    for (Entity entity : updateOrder) {
        auto& transform = m_SceneObj->GetComponent<TransformComponent>(entity);
        if (transform.IsValid()) {
            if (!transform.IsDirty()) continue;
        }

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

    // Sort by depth in ascending order (from shallow to deep)
    std::vector<Entity> sorted = m_DirtySet;
    std::sort(sorted.begin(), sorted.end(), [this](Entity a, Entity b) {
        auto& ra = m_SceneObj->GetComponent<RelationshipComponent>(a);
        auto& rb = m_SceneObj->GetComponent<RelationshipComponent>(b);
        return ra.GetDepth() < rb.GetDepth();
    });
    return sorted;
}

void TransformSystem::ComputeDepth(Entity entity, uint32_t depth) {
    auto& rel = m_SceneObj->GetComponent<RelationshipComponent>(entity);

    rel.SetDepth(depth);
    for (Entity child : rel.Children) {
        ComputeDepth(child, depth + 1);
    }
}

void TransformSystem::UpdateEntity(Entity entity) {
    auto& transform = m_SceneObj->GetComponent<TransformComponent>(entity);
    auto& rel       = m_SceneObj->GetComponent<RelationshipComponent>(entity);

    if (!transform.IsValid()) return;

    Matrix4 local = transform.GetLocalMatrix();

    if (rel.HasParent()) {
        auto& parentTransform = m_SceneObj->GetComponent<TransformComponent>(rel.Parent);
        if (parentTransform.IsValid() && !parentTransform.IsDirty()) {
            // Parent node already updated, use its world matrix
            transform.WorldMatrix = parentTransform.WorldMatrix * local;
        } else {
            // Parent node not yet updated (theoretically shouldn’t happen because sorting
            // Can recursively update the parent node
            UpdateEntity(rel.Parent);
            auto& updatedParent   = m_SceneObj->GetComponent<TransformComponent>(rel.Parent);
            transform.WorldMatrix = updatedParent.WorldMatrix * local;
        }
    } else {
        transform.WorldMatrix = local;
    }

    // Compute the normal matrix (inverse transpose of the 3x3 part)
    // Simplified here: directly take the 3x3 part and orthogonalize (if non-uniform scaling is
    // involved, use inverse transpose) Production code should use inverse transpose
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
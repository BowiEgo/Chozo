#include "TransformSystem.h"

#include "RelationshipComponent.h"
#include "Scene.h"
#include "TransformComponent.h"

void FTransformSystem::Update() {
    if (m_DirtySet.empty()) return;

    // Get update order (sort by depth to ensure parent nodes are processed before children)
    auto updateOrder = GetUpdateOrder();

    for (FEntity entity : updateOrder) {
        auto& transform = m_Scene->GetComponent<FTransformComponent>(entity);
        if (transform.IsValid()) {
            if (!transform.IsDirty()) continue;
        }

        UpdateEntity(entity);
        transform.ClearDirty();
    }

    m_DirtySet.clear();
}

std::vector<FEntity> FTransformSystem::GetUpdateOrder() {
    if (m_bNeedSort) {
        // Recalculate depth for all entities
        // 1. Find all root nodes (those with no parent)
        std::vector<FEntity> roots;
        auto view = m_Scene->View<FTransformComponent, FRelationshipComponent>();
        for (FEntity entity : view) {
            auto& rel = m_Scene->GetComponent<FRelationshipComponent>(entity);
            if (!rel.HasParent()) {
                roots.push_back(entity);
            }
        }

        // 2. DFS to compute depth
        for (FEntity root : roots) {
            ComputeDepth(root, 0);
        }
        m_bNeedSort = false;
    }

    // Sort by depth in ascending order (from shallow to deep)
    std::vector<FEntity> sorted = m_DirtySet;
    std::sort(sorted.begin(), sorted.end(), [this](FEntity a, FEntity b) {
        auto& ra = m_Scene->GetComponent<FRelationshipComponent>(a);
        auto& rb = m_Scene->GetComponent<FRelationshipComponent>(b);
        return ra.GetDepth() < rb.GetDepth();
    });
    return sorted;
}

void FTransformSystem::ComputeDepth(FEntity entity, uint32_t depth) {
    auto& rel = m_Scene->GetComponent<FRelationshipComponent>(entity);

    rel.SetDepth(depth);
    for (FEntity child : rel.Children) {
        ComputeDepth(child, depth + 1);
    }
}

void FTransformSystem::UpdateEntity(FEntity entity) {
    auto& transform = m_Scene->GetComponent<FTransformComponent>(entity);
    auto& rel       = m_Scene->GetComponent<FRelationshipComponent>(entity);

    if (!transform.IsValid()) return;

    FMatrix4 local = transform.GetLocalMatrix();

    if (rel.HasParent()) {
        auto& parentTransform = m_Scene->GetComponent<FTransformComponent>(rel.Parent);
        if (parentTransform.IsValid() && !parentTransform.IsDirty()) {
            // Parent node already updated, use its world matrix
            transform.WorldMatrix = parentTransform.WorldMatrix * local;
        } else {
            // Parent node not yet updated (theoretically shouldn’t happen because sorting
            // Can recursively update the parent node
            UpdateEntity(rel.Parent);
            auto& updatedParent   = m_Scene->GetComponent<FTransformComponent>(rel.Parent);
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

void FTransformSystem::MarkDirty(FEntity entity) {
    auto& transform = m_Scene->GetComponent<FTransformComponent>(entity);
    // if (transform.IsDirty()) return;

    transform.MarkDirty();
    m_DirtySet.push_back(entity);
    m_bNeedSort = true; // Depth may change, need to reorder before next update

    // Recursively mark all child nodes (optional, because children will detect dirty parent during
    // update and recompute)
    auto& rel = m_Scene->GetComponent<FRelationshipComponent>(entity);
    for (FEntity child : rel.Children) {
        MarkDirty(child);
    }
}

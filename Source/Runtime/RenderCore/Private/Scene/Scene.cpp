#include "Scene.h"
#include "NameComponent.h"
#include "RelationshipComponent.h"
#include "TransformComponent.h"

DEFINE_LOG_CATEGORY(LogScene);

// ===== Entity Management =====
FEntity FScene::CreateEntity(const std::string& name) {
    // Create entity in registry
    entt::entity handle = m_Registry.create();
    FEntity entity(handle);

    // Add default Transform component
    AddComponent<FTransformComponent>(entity);

    // Add Name component with provided name or generate default
    if (!name.empty()) {
        AddComponent<FNameComponent>(entity, name);
    } else {
        // Generate default name: "Entity_" + entity ID
        std::string defaultName = "Entity_" + std::to_string(static_cast<uint32_t>(handle));
        AddComponent<FNameComponent>(entity, defaultName);
    }

    // Add empty Relationship component (for hierarchy)
    AddComponent<FRelationshipComponent>(entity);

    // CZ_LOG(LogScene, Trace, "Created entity [{}] with name '{}'", static_cast<uint32_t>(handle),
    //        name.empty() ? "Unnamed" : name);

    return entity;
}

void FScene::DestroyEntity(FEntity entity) {
    if (!IsValid(entity)) {
        CZ_LOG(LogScene, Warning, "Attempted to destroy invalid entity");
        return;
    }

    entt::entity handle = entity.GetHandle();

    // Clean up hierarchy relationships before destruction
    if (HasComponent<FRelationshipComponent>(entity)) {
        auto& rel = GetComponent<FRelationshipComponent>(entity);

        // Remove this entity from parent's children list
        if (rel.Parent.IsValid() && HasComponent<FRelationshipComponent>(rel.Parent)) {
            auto& parentRel = GetComponent<FRelationshipComponent>(rel.Parent);
            auto& children = parentRel.Children;
            children.erase(std::remove(children.begin(), children.end(), entity), children.end());
        }

        // Orphan all children (set parent to null)
        for (FEntity child : rel.Children) {
            if (IsValid(child) && HasComponent<FRelationshipComponent>(child)) {
                GetComponent<FRelationshipComponent>(child).Parent = FEntity();
            }
        }
    }

    // Destroy the entity and all its components
    m_Registry.destroy(handle);

    // CZ_LOG(LogScene, Trace, "Destroyed entity [{}]", static_cast<uint32_t>(handle));
}

bool FScene::IsValid(FEntity entity) const {
    if (!entity.IsValid()) return false;

    entt::entity handle = entity.GetHandle();
    return m_Registry.valid(handle);
}

// ===== Relationship Management =====
FEntity FScene::GetParent(FEntity entity) {
    if (!IsValid(entity) || !HasComponent<FRelationshipComponent>(entity)) {
        return FEntity();
    }
    return GetComponent<FRelationshipComponent>(entity).Parent;
}

void FScene::SetParent(FEntity child, FEntity parent) {
    if (!IsValid(child)) {
        CZ_LOG(LogScene, Warning, "Cannot set parent: child is invalid");
        return;
    }

    // Handle removal from old parent
    if (HasComponent<FRelationshipComponent>(child)) {
        auto& childRel = GetComponent<FRelationshipComponent>(child);
        if (childRel.Parent.IsValid()) {
            // Remove from old parent's children list
            if (HasComponent<FRelationshipComponent>(childRel.Parent)) {
                auto& parentRel = GetComponent<FRelationshipComponent>(childRel.Parent);
                auto& children = parentRel.Children;
                children.erase(std::remove(children.begin(), children.end(), child),
                               children.end());
            }
        }
    }

    // Set new parent
    if (parent.IsValid() && IsValid(parent)) {
        // Ensure both have Relationship component
        if (!HasComponent<FRelationshipComponent>(child)) {
            AddComponent<FRelationshipComponent>(child);
        }
        if (!HasComponent<FRelationshipComponent>(parent)) {
            AddComponent<FRelationshipComponent>(parent);
        }

        auto& childRel = GetComponent<FRelationshipComponent>(child);
        auto& parentRel = GetComponent<FRelationshipComponent>(parent);

        childRel.Parent = parent;
        parentRel.Children.push_back(child);
    } else {
        // No parent (root node)
        if (HasComponent<FRelationshipComponent>(child)) {
            GetComponent<FRelationshipComponent>(child).Parent = FEntity();
        }
    }

    // CZ_LOG(LogScene, Trace, "Set parent for entity [{}] -> [{}]",
    //        static_cast<uint32_t>(child.GetHandle()),
    //        parent.IsValid() ? static_cast<uint32_t>(parent.GetHandle()) : 0);
}

std::vector<FEntity> FScene::GetChildren(FEntity entity) {
    if (!IsValid(entity) || !HasComponent<FRelationshipComponent>(entity)) {
        return {};
    }
    return GetComponent<FRelationshipComponent>(entity).Children;
}

// ===== Serialization =====
// void FScene::Serialize(FArchive& ar) {
//     // Serialize scene metadata
//     ar << m_Name;
//     ar << m_ID;

//     // Get all entities
//     std::vector<entt::entity> entities;
//     for (auto entity : m_Registry.storage<entt::entity>()) {
//         entities.push_back(entity);
//     }

//     // Serialize entity count
//     uint32_t entityCount = static_cast<uint32_t>(entities.size());
//     ar << entityCount;

//     // Serialize each entity and its components
//     for (entt::entity handle : entities) {
//         FEntity entity(handle);

//         // Serialize entity ID
//         uint32_t entityId = static_cast<uint32_t>(handle);
//         ar << entityId;

//         // Serialize components (implement per component type)
//         // This would require component type registration system
//         // Simplified example:
//         if (HasComponent<FTransformComponent>(entity)) {
//             ar << GetComponent<FTransformComponent>(entity);
//         }
//         if (HasComponent<FNameComponent>(entity)) {
//             ar << GetComponent<FNameComponent>(entity);
//         }
//         // ... other components
//     }
// }

// void FScene::Deserialize(FArchive& ar) {
//     // Deserialize scene metadata
//     ar >> m_Name;
//     ar >> m_ID;

//     // Deserialize entity count
//     uint32_t entityCount;
//     ar >> entityCount;

//     // Clear existing registry
//     m_Registry.clear();

//     // Deserialize each entity
//     for (uint32_t i = 0; i < entityCount; ++i) {
//         uint32_t entityId;
//         ar >> entityId;

//         FEntity entity = CreateEntity();

//         // Deserialize components (implement per component type)
//         // This would require component type registration system
//         // Simplified example:
//         if (ar.HasData<FTransformComponent>()) {
//             FTransformComponent transform;
//             ar >> transform;
//             AddComponent<FTransformComponent>(entity, transform);
//         }
//         if (ar.HasData<FNameComponent>()) {
//             FNameComponent name;
//             ar >> name;
//             GetComponent<FNameComponent>(entity) = name;
//         }
//         // ... other components
//     }
// }

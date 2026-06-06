#include <Runtime/RenderCore/Components/Components.hpp>
#include <Runtime/RenderCore/MeshRegistry.hpp>
#include <Runtime/RenderCore/Scene/Scene.hpp>

#include "SceneECS.hpp" // IWYU pragma: keep

namespace CZ {

DEFINE_HANDLE_DESTROY(SceneObj)

SceneObj::SceneObj() : m_Impl(CZ_NEW(MEMORY_USAGE_SCENE, SceneImpl)) {}

void SceneObj::Update(float deltaTime) {
    // m_TransformSystem.Update();

    // auto view = View<MeshComponent, TransformComponent>();
    // for (auto entity : view) {
    //     auto& meshComp = view.get<MeshComponent>(entity);
    //     if (meshComp.IsValid()) {
    //         auto mesh = MeshRegistry::Get().GetMesh(meshComp.MeshHandle);
    //         if (meshComp.IsDirty()) {
    //             meshComp.UpdateMesh();
    //             mesh->Upload();
    //             CZ_LOG(LogScene, Trace, "UpdateMesh");
    //         }
    //     }
    // }
}

void SceneObj::Draw(CommandList cmdList, GraphicsBuffer cameraBuffer) {}

// ===== Entity Management =====
Entity SceneObj::CreateEntity(const std::string& name) {
    // Create entity in registry
    entt::entity handle = m_Impl->m_Registry.create();
    auto entity         = EntityFromEntt(handle);

    // Add default Transform component
    AddComponent<TransformComponent>(entity);

    // Add Name component with provided name or generate default
    if (!name.empty()) {
        AddComponent<NameComponent>(entity, name);
    } else {
        // Generate default name: "Entity_" + entity ID
        std::string defaultName = "Entity_" + std::to_string(static_cast<uint32_t>(handle));
        AddComponent<NameComponent>(entity, defaultName);
    }

    // Add empty Relationship component (for hierarchy)
    AddComponent<RelationshipComponent>(entity);

    // CZ_LOG(LogScene, Trace, "Created entity [{}] with name '{}'", static_cast<uint32_t>(handle),
    //        name.empty() ? "Unnamed" : name);

    return entity;
}

void SceneObj::DestroyEntity(Entity entity) {
    if (!IsValid(entity)) {
        CZ_LOG(LogScene, Warning, "Attempted to destroy invalid entity");
        return;
    }

    // Clean up hierarchy relationships before destruction
    if (HasComponent<RelationshipComponent>(entity)) {
        auto& rel = GetComponent<RelationshipComponent>(entity);

        // Remove this entity from parent's children list
        if (rel.Parent.IsValid() && HasComponent<RelationshipComponent>(rel.Parent)) {
            auto& parentRel = GetComponent<RelationshipComponent>(rel.Parent);
            auto& children  = parentRel.Children;
            children.erase(std::remove(children.begin(), children.end(), entity), children.end());
        }

        // Orphan all children (set parent to null)
        for (Entity child : rel.Children) {
            if (IsValid(child) && HasComponent<RelationshipComponent>(child)) {
                GetComponent<RelationshipComponent>(child).Parent = Entity();
            }
        }
    }

    // Destroy the entity and all its components
    m_Impl->m_Registry.destroy(EntityToEntt(entity));

    // CZ_LOG(LogScene, Trace, "Destroyed entity [{}]", static_cast<uint32_t>(handle));
}

bool SceneObj::IsValid(Entity entity) const {
    if (!entity.IsValid()) return false;

    return m_Impl->m_Registry.valid(EntityToEntt(entity));
}

// ===== Relationship Management =====
Entity SceneObj::GetParent(Entity entity) {
    if (!IsValid(entity) || !HasComponent<RelationshipComponent>(entity)) {
        return Entity();
    }
    return GetComponent<RelationshipComponent>(entity).Parent;
}

void SceneObj::SetParent(Entity child, Entity parent) {
    if (!IsValid(child)) {
        CZ_LOG(LogScene, Warning, "Cannot set parent: child is invalid");
        return;
    }

    // Handle removal from old parent
    if (HasComponent<RelationshipComponent>(child)) {
        auto& childRel = GetComponent<RelationshipComponent>(child);
        if (childRel.Parent.IsValid()) {
            // Remove from old parent's children list
            if (HasComponent<RelationshipComponent>(childRel.Parent)) {
                auto& parentRel = GetComponent<RelationshipComponent>(childRel.Parent);
                auto& children  = parentRel.Children;
                children.erase(std::remove(children.begin(), children.end(), child),
                               children.end());
            }
        }
    }

    // Set new parent
    if (parent.IsValid() && IsValid(parent)) {
        // Ensure both have Relationship component
        if (!HasComponent<RelationshipComponent>(child)) {
            AddComponent<RelationshipComponent>(child);
        }
        if (!HasComponent<RelationshipComponent>(parent)) {
            AddComponent<RelationshipComponent>(parent);
        }

        auto& childRel  = GetComponent<RelationshipComponent>(child);
        auto& parentRel = GetComponent<RelationshipComponent>(parent);

        childRel.Parent = parent;
        parentRel.Children.push_back(child);
    } else {
        // No parent (root node)
        if (HasComponent<RelationshipComponent>(child)) {
            GetComponent<RelationshipComponent>(child).Parent = Entity();
        }
    }

    // CZ_LOG(LogScene, Trace, "Set parent for entity [{}] -> [{}]",
    //        static_cast<uint32_t>(child.GetHandle()),
    //        parent.IsValid() ? static_cast<uint32_t>(parent.GetHandle()) : 0);
}

std::vector<Entity> SceneObj::GetChildren(Entity entity) {
    if (!IsValid(entity) || !HasComponent<RelationshipComponent>(entity)) {
        return {};
    }
    return GetComponent<RelationshipComponent>(entity).Children;
}

void SceneObj::SetTransform(Entity entity, const TransformParams params) {
    bool hasTransformComp = HasComponent<TransformComponent>(entity);
    auto& comp            = hasTransformComp ? GetComponent<TransformComponent>(entity)
                                             : AddComponent<TransformComponent>(entity);
    comp.SetTransformParams(params);

    m_TransformSystem.MarkDirty(entity);
}

void SceneObj::SetMesh(Entity entity, const MeshParams params) {
    // bool hasMeshComp = HasComponent<MeshComponent>(entity);
    // auto& comp =
    //     hasMeshComp ? GetComponent<MeshComponent>(entity) : AddComponent<MeshComponent>(entity);
    // comp.SetMeshParamsWrapper(params);
}

template <typename T> inline entt::id_type GetComponentTypeID() {
    return entt::type_id<T>().hash();
}

#define INSTANTIATE_TEMPLATES(ComponentType)                                                       \
    template ComponentType& SceneObj::GetComponent<ComponentType>(Entity);                         \
    template bool SceneObj::HasComponent<ComponentType>(Entity) const;                             \
    template void SceneObj::RemoveComponent<ComponentType>(Entity);

INSTANTIATE_TEMPLATES(NameComponent)
INSTANTIATE_TEMPLATES(TransformComponent)
// INSTANTIATE_TEMPLATES(MeshComponent)

Scene Scene::Create() {
    auto obj = CZ_NEW(MEMORY_USAGE_ASSET, SceneObj);
    return Scene(obj);
}

} // namespace CZ
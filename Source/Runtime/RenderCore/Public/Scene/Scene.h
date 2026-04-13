#pragma once

#include "CoreMinimal.h"
#include "RenderCoreExport.h"

#include "Asset.h"
#include "Entity.h"
#include "MeshParams.h"
#include "RHICommandList.h"
#include "TransformParams.h"
#include "TransformSystem.h"

#include "entt/entt.hpp"

DECLARE_LOG_CATEGORY_EXTERN(LogScene, Info);

class RENDER_CORE_API FScene : public IAsset {
public:
    FScene()  = default;
    ~FScene() = default;

    void Draw(IRHICommandList* cmdList);

    // ----- Entity Management -----
    FEntity CreateEntity(const std::string& name = "");
    void DestroyEntity(FEntity entity);
    bool IsValid(FEntity entity) const;

    // ----- Relationship -----
    FEntity GetParent(FEntity entity);
    void SetParent(FEntity child, FEntity parent);
    std::vector<FEntity> GetChildren(FEntity entity);

    // ----- Transform -----
    void SetTransform(FEntity entity, const FTransformParams& params);

    // ----- Mesh -----
    void SetMesh(FEntity entity, const FMeshParams& params);

    // ===== Component Operations =====
    template <typename T, typename... Args> T& AddComponent(FEntity entity, Args&&... args) {
        CZ_CORE_ASSERT(IsValid(entity), "Cannot add component to invalid entity");
        CZ_CORE_ASSERT(!HasComponent<T>(entity), "Component already exists");

        entt::entity handle = entity.GetHandle();
        return m_Registry.emplace<T>(handle, std::forward<Args>(args)...);
    }

    template <typename T> void RemoveComponent(FEntity entity) {
        CZ_CORE_ASSERT(IsValid(entity), "Cannot remove component from invalid entity");
        CZ_CORE_ASSERT(HasComponent<T>(entity), "Component does not exist");

        entt::entity handle = entity.GetHandle();
        m_Registry.erase<T>(handle);
    }

    template <typename T> T& GetComponent(FEntity entity) {
        CZ_CORE_ASSERT(IsValid(entity), "Cannot get component from invalid entity");
        CZ_CORE_ASSERT(HasComponent<T>(entity), "Component does not exist");

        entt::entity handle = entity.GetHandle();
        return m_Registry.get<T>(handle);
    }

    template <typename T> bool HasComponent(FEntity entity) const {
        if (!IsValid(entity)) return false;

        entt::entity handle = entity.GetHandle();
        return m_Registry.all_of<T>(handle);
    }

    // ----- Query System -----
    template <typename... Components> auto View() { return m_Registry.view<Components...>(); }

    template <typename... Components> auto View() const { return m_Registry.view<Components...>(); }

    // ----- Serialization -----
    // void Serialize(FArchive& ar) override;
    // void Deserialize(FArchive& ar) override;

private:
    entt::registry m_Registry;
    std::string m_Name;

    FTransformSystem m_TransformSystem{ this };
};
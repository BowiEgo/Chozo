#pragma once

#include <Runtime/RHI/GraphicsBuffer.hpp>
#include <Runtime/RenderCore/Asset.hpp>
#include <Runtime/RenderCore/Components/TransformParams.hpp>
// #include <Runtime/RenderCore/Material.hpp>
#include <Runtime/RenderCore/Mesh.hpp>
#include <Runtime/RenderCore/MeshParams.hpp>
#include <Runtime/RenderCore/Scene/Entity.hpp>
#include <Runtime/RenderCore/Scene/TransformSystem.hpp>

namespace CZ {

struct SceneImpl;

struct ScenePushConstants {
    Matrix4 ModelMatrix;
    Matrix3 NormalMatrix;
};

struct RenderData {
    // Material Material;
    ScenePushConstants PushConstants;
    Mesh Mesh;
};

struct SceneObj {
    Scope<SceneImpl> m_Impl;
    std::string m_Name;
    TransformSystem m_TransformSystem{ this };

    SceneObj();
    ~SceneObj() = default;

    void Update(float deltaTime);
    Entity CreateEntity(const std::string& name = "");
    void DestroyEntity(Entity entity);
    bool IsValid(Entity entity) const;
    Entity GetParent(Entity entity);
    void SetParent(Entity child, Entity parent);
    std::vector<Entity> GetChildren(Entity entity);
    void SetTransform(Entity entity, const TransformParams params);
    void SetMesh(Entity entity, const MeshParams props);

    std::vector<RenderData> GetRenderDatas();

    // // ===== Component Operations =====
    template <typename T, typename... Args> T& AddComponent(Entity entity, Args&&... args);

    template <typename T> void RemoveComponent(Entity entity);

    template <typename T> T& GetComponent(Entity entity);

    template <typename T> bool HasComponent(Entity entity) const;

    // // ----- Query System -----
    template <typename... Components> auto View();

    template <typename... Components> auto View() const;

    // ----- Serialization -----
    // void Serialize(FArchive& ar) override;
    // void Deserialize(FArchive& ar) override;
};

class Scene : public Asset<struct SceneObj> {
public:
    static Scene Create();

    Scene() = default;
    explicit Scene(SceneObj* obj) : Asset<SceneObj>(obj) {}
    virtual ~Scene() = default;

    AssetType GetType() const override { return AssetType::Scene; }

    const std::string GetName() const override { return m_Obj->m_Name; }
};

template <> struct AssetTraits<SceneObj> {
    using AssetClass = Scene;
};

} // namespace CZ

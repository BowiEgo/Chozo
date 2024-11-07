#pragma once

#include "czpch.h"

#include "entt.hpp"

#include "Chozo/Core/UUID.h"
#include "Chozo/Core/Timestep.h"
#include "Chozo/Renderer/EditorCamera.h"
#include "Chozo/Renderer/Environment.h"
#include "Chozo/Renderer/Pipeline.h"

namespace Chozo {

    class Entity;
    class SceneRenderer;

	using EntityMap = std::unordered_map<UUID, Entity>;

    class Scene : public Asset
    {
    public:
        Scene();
        ~Scene();

        static Ref<Scene> Create() { return Ref<Scene>::Create(); }

        static AssetType GetStaticType() { return AssetType::Scene; }
		virtual AssetType GetAssetType() const override { return GetStaticType(); }

        Entity CreateEntity(const std::string& name = std::string());
    	Entity CreateChildEntity(Entity parent, const std::string& name);
        Entity CreateEntityWithUUID(UUID uuid, const std::string& name = std::string());

        Entity GetEntityWithUUID(UUID uuid);

        glm::mat4 GetWorldSpaceTransformMatrix(Entity entity);

        void SortEntities();
        void DestroyEntity(Entity entity);
        bool EntityExists(entt::entity entity);

        Entity InstantiateMesh(Ref<Mesh> mesh);
	    void BuildMeshEntityHierarchy(Entity parent, Ref<Mesh> mesh, const MeshNode& node);

        entt::registry& Reg() { return m_Registry; }

        void OnUpdateEditor(Timestep ts);
        void OnUpdateRuntime(Timestep ts);
        void OnRenderEditor(Ref<SceneRenderer> renderer, Timestep ts, EditorCamera& camera);
        void OnRenderRuntime(Ref<SceneRenderer> renderer, Timestep ts);
        void OnViewportResize(uint32_t width, uint32_t height);

        void PrepareRender(Ref<SceneRenderer> renderer);
        void SubmitMeshes(Ref<SceneRenderer> renderer);

        Entity GetPrimaryCameraEntity();

        Ref<Environment> GetEnvironment() { return m_Environment; }
    private:
        template<typename T>
        void OnComponentAdded(Entity entity, T& component);
    private:
        entt::registry m_Registry;
		EntityMap m_EntityIDMap;

        uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;

		Ref<Environment> m_Environment;
		float m_EnvironmentIntensity = 0.0f;
		float m_SkyboxLod = 1.0f;

        friend class Entity;
        friend class SceneSerializer;
        friend class SceneHierarchyPanel;
        friend class PropertiesPanel;
		friend class SceneRenderer;
    };
}

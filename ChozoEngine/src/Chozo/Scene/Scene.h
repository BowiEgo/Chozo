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

    class Scene : public RefCounted
    {
    public:
        Scene();
        ~Scene();

        Entity CreateEntity(const std::string& name = std::string());
        Entity CreateEntityWithUUID(UUID uuid, const std::string& name = std::string());
        void DestroyEntity(Entity entity);
        bool EntityExists(entt::entity entity);

        // TEMP
        entt::registry& Reg() { return m_Registry; }

        void OnUpdateEditor(Timestep ts, EditorCamera& camera);
        void OnUpdateRuntime(Timestep ts);
        void OnViewportResize(uint32_t width, uint32_t height);

        void PrepareRender(Ref<SceneRenderer> renderer);
        void SubmitMeshes(Ref<SceneRenderer> renderer);

        void RenderToBuffer(EditorCamera& camera, SharedBuffer& dest);

        Entity GetPrimaryCameraEntity();

        Ref<Environment> GetEnvironment() { return m_Environment; }
    private:
        template<typename T>
        void OnComponentAdded(Entity entity, T& component);
    public:
        entt::registry m_Registry;

        friend class Entity;
        friend class SceneSerializer;
        friend class SceneHierarchyPanel;
        friend class PropertiesPanel;
    private:
        uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;

		Ref<Environment> m_Environment;
		float m_EnvironmentIntensity = 0.0f;
		float m_SkyboxLod = 1.0f;

		friend class SceneRenderer;
    };
}

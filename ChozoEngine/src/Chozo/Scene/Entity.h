#pragma once

#include "Scene.h"
#include "Components.h"
#include "Chozo/Core/UUID.h"

#include "entt.hpp"

namespace Chozo
{
    class Entity
    {
    public:
        Entity() = default;
        Entity(entt::entity handle, Scene* scene);
        Entity(const Entity& other) = default;
        ~Entity() {};

        template<typename T, typename... Args>
        T& AddComponent(Args&&... args)
        {
            CZ_CORE_ASSERT(!HasComponent<T>(), "Entity already has this component!");
            T& component = m_Scene->m_Registry.emplace<T>(m_EntityHandle, std::forward<Args>(args)...);
            m_Scene->OnComponentAdded(*this, component);
            return component;
        }

        template<typename T>
        void RemoveComponent()
        {
            CZ_CORE_ASSERT(HasComponent<T>(), "Entity does not have this component!");
            m_Scene->m_Registry.erase<T>(m_EntityHandle);
        }

        template<typename T>
        T& GetComponent()
        {
            CZ_CORE_ASSERT(HasComponent<T>(), "Entity does not have this component!");
            return m_Scene->m_Registry.get<T>(m_EntityHandle);
        }

        template<typename T>
        bool HasComponent()
        {
            return m_Scene->m_Registry.all_of<T>(m_EntityHandle);
        }

        UUID GetUUID() { return GetComponent<IDComponent>().ID; }

        operator bool() const { return m_EntityHandle != entt::null; }
        operator entt::entity() const { return m_EntityHandle; }
        operator uint64_t() const { return (uint64_t)m_EntityHandle; }
        bool operator==(const Entity& other) const {
            return m_EntityHandle == other.m_EntityHandle && m_Scene == other.m_Scene;
        }

        bool operator!=(const Entity& other) const {
            return !(*this == other);
        }
    public:
        friend class MeshComponent;
    private:
        entt::entity m_EntityHandle{ entt::null };
        Scene* m_Scene = nullptr;
    };
}

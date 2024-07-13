#pragma once

#include "entt.hpp"

#include "Scene.h"

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
        T& AddCompoent(Args&&... args)
        {
            CZ_CORE_ASSERT(!HasComponent<T>(), "Entity already has this component!");
            return m_Scene->m_Registry.emplace<T>(m_EntityHandle, std::forward<Args>(args)...);
        }

        template<typename T>
        void RemoveCompoent()
        {
            CZ_CORE_ASSERT(HasComponent<T>(), "Entity does not have this component!");
            m_Scene->m_Registry.remove<T>(m_EntityHandle);
        }

        template<typename T>
        T& GetCompoent()
        {
            CZ_CORE_ASSERT(HasComponent<T>(), "Entity does not have this component!");
            return m_Scene->m_Registry.get<T>(m_EntityHandle);
        }

        template<typename T>
        bool HasComponent()
        {
            return m_Scene->m_Registry.all_of<T>(m_EntityHandle);
        }

        operator bool() const { return m_EntityHandle != entt::null; }
    private:
        entt::entity m_EntityHandle;
        Scene* m_Scene = nullptr;
    };
}

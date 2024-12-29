#pragma once

#include "Scene.h"
#include "Components.h"
#include "Chozo/Core/UUID.h"

#include "entt.hpp"
#include "spdlog/spdlog.h"
#include "spdlog/fmt/ostr.h"

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
        const T& GetComponent() const
        {
            CZ_CORE_ASSERT(HasComponent<T>(), "Entity does not have this component!");
            return m_Scene->m_Registry.get<T>(m_EntityHandle);
        }

        template<typename T>
        bool HasComponent() const
        {
            return m_Scene->m_Registry.all_of<T>(m_EntityHandle);
        }

        UUID GetUUID() { return GetComponent<IDComponent>().ID; }

        operator bool() const { return m_EntityHandle != entt::null; }
        operator entt::entity() const { return m_EntityHandle; }
		operator uint32_t () const { return (uint32_t)m_EntityHandle; }
        operator uint64_t() const { return (uint64_t)m_EntityHandle; }
        bool operator==(const Entity& other) const {
            return m_EntityHandle == other.m_EntityHandle && m_Scene == other.m_Scene;
        }

        bool operator!=(const Entity& other) const
        {
            return !(*this == other);
        }

        TransformComponent& Transform() { return GetComponent<TransformComponent>(); }
        glm::mat4 Transform() const { return GetComponent<TransformComponent>().GetTransform(); }

		void SetParent(Entity parent);
		Entity GetParent() { return m_Scene->GetEntityWithUUID(GetParentUUID()); }
		UUID GetParentUUID() { return GetComponent<RelationshipComponent>().ParentHandle; }
		void SetParentUUID(const UUID uuid) { GetComponent<RelationshipComponent>().ParentHandle = uuid; }

		std::vector<UUID>& Children() { return GetComponent<RelationshipComponent>().Children; }
		bool RemoveChild(Entity child);
        void Destroy();
    public:
        friend struct MeshComponent;
    private:
        entt::entity m_EntityHandle{ entt::null };
        Scene* m_Scene = nullptr;
    };

    inline std::ostream& operator<<(std::ostream& os, const Entity& entity)
    {
        os << (char)(uint64_t)entity;
        return os;
    }

}

#if FMT_VERSION >= 90000
template <>
struct fmt::formatter<Chozo::Entity> {
    // This parses the format specifications (if any) for Entity, which we don't use in this case
    constexpr auto parse(const format_parse_context& ctx) { return ctx.begin(); }

    // This formats the Entity for output
    template <typename FormatContext>
    auto format(const Chozo::Entity& entity, FormatContext& ctx) {
        // Use the Entity's conversion operator to uint64_t, and then format it
        return format_to(ctx.out(), "{}", (uint64_t)entity);
    }
};
#endif

#pragma once
#include <Runtime/RenderCore/Components/Components.hpp>
#include <Runtime/RenderCore/MeshRegistry.hpp>
#include <Runtime/RenderCore/Scene/Scene.hpp>

#include "EntityImpl.hpp"
#include "SceneImpl.hpp"

#include <entt/entt.hpp>

namespace CZ {

template <typename T, typename... Args> T& SceneObj::AddComponent(Entity entity, Args&&... args) {
    return m_Impl->m_Registry.emplace<T>(EntityToEntt(entity), std::forward<Args>(args)...);
}

template <typename T> void SceneObj::RemoveComponent(Entity entity) {
    m_Impl->m_Registry.remove<T>(EntityToEntt(entity));
}

template <typename T> T& SceneObj::GetComponent(Entity entity) {
    return m_Impl->m_Registry.get<T>(EntityToEntt(entity));
}

template <typename T> bool SceneObj::HasComponent(Entity entity) const {
    return m_Impl->m_Registry.all_of<T>(EntityToEntt(entity));
}

template <typename... Components> auto SceneObj::View() {
    return m_Impl->m_Registry.view<Components...>();
}

template <typename... Components> auto SceneObj::View() const {
    return m_Impl->m_Registry.view<Components...>();
}

} // namespace CZ
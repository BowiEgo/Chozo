#pragma once
#include <Runtime/RenderCore/Scene/Entity.hpp>
#include <Runtime/RenderCore/Scene/TransformSystem.hpp>

#include <entt/entt.hpp>

namespace CZ {

struct SceneImpl {
    entt::registry m_Registry;
};

} // namespace CZ
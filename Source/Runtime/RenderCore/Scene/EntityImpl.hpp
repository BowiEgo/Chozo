#pragma once

#include <Runtime/RenderCore/Scene/Entity.hpp>

#include <entt/entt.hpp>

namespace CZ {

inline entt::entity EntityToEntt(Entity entity) {
    return static_cast<entt::entity>(entity.GetHandle());
}

inline Entity EntityFromEntt(entt::entity handle) { return Entity(static_cast<uint32_t>(handle)); }

} // namespace CZ
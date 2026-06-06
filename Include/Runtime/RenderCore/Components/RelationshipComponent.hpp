#pragma once

#include <Runtime/RenderCore/Scene/Entity.hpp>

namespace CZ {

struct RelationshipComponent {
    Entity Parent;
    std::vector<Entity> Children;
    uint32_t Depth = 0;

    bool HasParent() const { return Parent.IsValid() && Parent != Entity(); }
    bool HasChildren() const { return !Children.empty(); }

    void AddChild(Entity child) { Children.push_back(child); }
    void RemoveChild(Entity child) {
        Children.erase(std::remove(Children.begin(), Children.end(), child), Children.end());
    }

    void SetDepth(uint32_t depth) { Depth = depth; }
    uint32_t GetDepth() const { return Depth; }
};

} // namespace CZ

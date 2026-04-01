#pragma once

#include "Entity.h"

struct FRelationshipComponent {
    FEntity Parent;
    std::vector<FEntity> Children;
    uint32_t Depth = 0;

    bool HasParent() const { return Parent.IsValid() && Parent != FEntity(); }
    bool HasChildren() const { return !Children.empty(); }

    void AddChild(FEntity child) { Children.push_back(child); }
    void RemoveChild(FEntity child) {
        Children.erase(std::remove(Children.begin(), Children.end(), child), Children.end());
    }

    void SetDepth(uint32_t depth) { Depth = depth; }
    uint32_t GetDepth() const { return Depth; }
};

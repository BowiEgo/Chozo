#pragma once

#include "Entity.h"

struct FRelationshipComponent {
    FEntity Parent;
    std::vector<FEntity> Children;

    bool HasParent() const { return Parent.IsValid(); }
    bool HasChildren() const { return !Children.empty(); }

    void AddChild(FEntity child) { Children.push_back(child); }

    void RemoveChild(FEntity child) {
        Children.erase(std::remove(Children.begin(), Children.end(), child), Children.end());
    }
};

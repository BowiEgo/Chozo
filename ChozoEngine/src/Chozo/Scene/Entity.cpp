#include "Entity.h"

namespace Chozo {

    Entity::Entity(entt::entity handle, Scene *scene)
        : m_EntityHandle(handle), m_Scene(scene)
    {
    }

    void Entity::SetParent(Entity parent)
    {
        Entity currrentParent = GetParent();
        if (currrentParent == parent)
            return;

        if (currrentParent)
            currrentParent.RemoveChild(*this);

        SetParentUUID(parent.GetUUID());

        if (parent)
        {
            auto& parentChildren = parent.Children();
            UUID uuid = GetUUID();
            if (std::find(parentChildren.begin(), parentChildren.end(), uuid) == parentChildren.end())
                parentChildren.emplace_back(uuid);
        }
    }
    
    bool Entity::RemoveChild(Entity child)
    {
        UUID childID = child.GetUUID();
        auto& children = Children();
        auto it = std::find(children.begin(), children.end(), childID);
        if (it != children.end())
        {
            children.erase(it);
            return true;
        }

        return false;
    }
}

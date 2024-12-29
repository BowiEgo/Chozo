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
        const UUID childID = child.GetUUID();
        auto& children = Children();
        if (const auto it = std::find(children.begin(), children.end(), childID); it != children.end())
        {
            children.erase(it);
            return true;
        }

        return false;
    }

    void Entity::Destroy()
    {
        if (m_EntityHandle == entt::null)
            return;

        auto& children = Children();
        for (const auto& childID : children)
        {
            auto child = m_Scene->GetEntityWithUUID(childID);
            child.Destroy();
        }

        if (Entity currentParent = GetParent())
            currentParent.RemoveChild(*this);

        m_Scene->m_EntityIDMap.erase(GetUUID());
        m_Scene->m_Registry.destroy(*this);
        m_EntityHandle = entt::null;
    }
}

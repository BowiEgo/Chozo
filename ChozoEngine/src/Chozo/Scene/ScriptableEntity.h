#pragma once

#include "Entity.h"

namespace Chozo {

    class Entity;

    class ScriptableEntity : public RefCounted
    {
    public:
        virtual ~ScriptableEntity() = default;

        template<typename T>
        T& GetComponent()
        {
            return m_Entity.GetComponent<T>();
        }
    protected:
        virtual void OnCreate() {}
        virtual void OnDestroy() {}
        virtual void OnUpdate(TimeStep ts) {}
    private:
        Entity m_Entity;
        friend class Scene;
    };
}

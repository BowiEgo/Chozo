#pragma once

#include "Entity.h"

namespace Chozo {

    class ScriptableEntity
    {
    public:
        virtual ~ScriptableEntity() = default;

        template<typename T>
        T& GetCompoent()
        {
            return m_Entity.GetCompoent<T>();
        }
    protected:
        virtual void OnCreate() {}
        virtual void OnDestroy() {}
        virtual void OnUpdate(Timestep ts) {}
    private:
        Entity m_Entity;
        friend class Scene;
    };
}
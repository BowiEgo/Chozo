#pragma once

#include "czpch.h"

#include "entt.hpp"

#include "Chozo/Core/Timestep.h"

namespace Chozo {

    class Scene
    {
    public:
        Scene();
        ~Scene();

        entt::entity CreateEntity();

        // TEMP
        entt::registry& Reg() { return m_Registry; }

        void OnUpdate(Timestep ts);
    private:
        entt::registry m_Registry;
    };
}


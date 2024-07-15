#pragma once

#include "czpch.h"

#include "entt.hpp"

#include "Chozo/Core/Timestep.h"

namespace Chozo {

    class Entity;

    class Scene
    {
    public:
        Scene();
        ~Scene();

        Entity CreateEntity(const std::string& name = std::string());

        // TEMP
        entt::registry& Reg() { return m_Registry; }

        void OnUpdate(Timestep ts);
        void OnViewportResize(uint32_t width, uint32_t height);
    public:
        entt::registry m_Registry;

        friend class Entity;
        friend class SceneHierarchy;
    private:
        uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;
    };
}


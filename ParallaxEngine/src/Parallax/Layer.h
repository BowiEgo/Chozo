#pragma once

#include "Parallax/Core.h"
#include "Parallax/Events/Event.h"
#include "Parallax/Core/Timestep.h"

namespace Parallax {
    class PARALLAX_API Layer
    {
    public:
        Layer(const std::string& name = "Layer");
        virtual ~Layer();

        virtual void OnAttach() {}
        virtual void OnDetach() {}
        virtual void OnUpdate(Timestep timestep) {}
        virtual void OnImGuiRender() {}
        virtual void OnEvent(Event& event) {}

        inline const std::string& GetName() const { return m_DebugName; }
    protected:
        std::string m_DebugName;
    };
}
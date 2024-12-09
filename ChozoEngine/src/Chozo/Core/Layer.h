#pragma once

#include "Chozo/Events/Event.h"
#include "Chozo/Core/Timestep.h"

namespace Chozo {

    class Layer
    {
    public:
        explicit Layer(std::string name = "Layer");
        virtual ~Layer();

        virtual void OnAttach() {}
        virtual void OnDetach() {}
        virtual void OnUpdate(TimeStep timeStep) {}
        virtual void OnImGuiRender() {}
        virtual void OnEvent(Event& event) {}

        const std::string& GetName() const { return m_DebugName; } // NOLINT
    protected:
        std::string m_DebugName;
    };
}
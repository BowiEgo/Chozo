#pragma once

#include "Chozo/Core/Base.h"
#include "Chozo/Events/Event.h"
#include "Chozo/Core/Timestep.h"

namespace Chozo {
    class CHOZO_API Layer
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
#pragma once

#include "CoreExport.h"
#include "Event.h"
#include "Timestep.h"

class CORE_API ILayer {
public:
    explicit ILayer(std::string name = "Layer");
    virtual ~ILayer();

    virtual void OnAttach() {}
    virtual void OnDetach() {}
    virtual void OnUpdate(FTimeStep timeStep) {}
    virtual void OnImGuiRender() {}
    virtual void OnEvent(IEvent& event) {}

    const std::string& GetName() const { return m_DebugName; } // NOLINT
protected:
    std::string m_DebugName;
};
#pragma once

#include "Event.h"
#include "KeyEvent.h"

#include "CoreExport.h"

class CORE_API ILayer {
public:
    explicit ILayer(std::string name = "Layer");
    virtual ~ILayer();

    virtual void OnAttach() {}
    virtual void OnDetach() {}
    virtual void OnUpdate(float deltaTime) {}
    virtual void OnImGuiRender() {}
    virtual void OnEvent(IEvent& event) {}
    virtual bool OnKeyPressed(FKeyPressedEvent& e) { return false; }
    // virtual bool OnMouseButtonPressed(FMouseButtonPressedEvent& e) { return false; }
    // virtual bool OnMouseButtonReleased(FMouseButtonReleasedEvent& e) { return false; }

    const std::string& GetName() const { return m_DebugName; } // NOLINT
protected:
    std::string m_DebugName;
};
#pragma once

#include <Core/Event/Event.hpp>
#include <Core/Event/KeyEvent.hpp>
#include <Core/Header/Handle.hpp>
#include <Core/Memory/Memory.hpp>

namespace CZ {

class Layer {
public:
    Layer()          = default;
    virtual ~Layer() = default;

    virtual void OnAttach()                       = 0;
    virtual void OnDetach()                       = 0;
    virtual void OnUpdate(float deltaTime)        = 0;
    virtual void OnRender()                       = 0;
    virtual void OnEvent(Event& e)                = 0;
    virtual bool OnKeyPressed(KeyPressedEvent& e) = 0;
    // virtual bool OnMouseButtonPressed(MouseButtonPressedEvent& e) { return false; }
    // virtual bool OnMouseButtonReleased(MouseButtonReleasedEvent& e) { return false; }

    const std::string& GetName() const { return m_DebugName; } // NOLINT

    static void Destroy() {}

protected:
    std::string m_DebugName;
};

} // namespace CZ
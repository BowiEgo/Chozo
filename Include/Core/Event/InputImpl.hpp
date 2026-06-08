#pragma once

#include <Core/Event/KeyCodes.hpp>

namespace CZ {

class InputImpl {
public:
    virtual ~InputImpl() = default;

    virtual bool IsKeyPressed(KeyCode keycode)            = 0;
    virtual bool IsMouseButtonPressed(MouseButton button) = 0;
    virtual std::pair<float, float> GetMousePosition()    = 0;
    virtual float GetMouseX()                             = 0;
    virtual float GetMouseY()                             = 0;
};

} // namespace CZ
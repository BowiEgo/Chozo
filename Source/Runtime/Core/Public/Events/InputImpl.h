#pragma once

#include "KeyCodes.h"

class IInputImpl {
public:
    virtual ~IInputImpl() = default;

    virtual bool IsKeyPressed(EKeyCode keycode) = 0;
    virtual bool IsMouseButtonPressed(EMouseButton button) = 0;
    virtual std::pair<float, float> GetMousePosition() = 0;
    virtual float GetMouseX() = 0;
    virtual float GetMouseY() = 0;
};
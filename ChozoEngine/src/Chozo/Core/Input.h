#pragma once

#include "KeyCodes.h"

namespace Chozo {

    class Input
    {
    public:
        static bool IsKeyPressed(const KeyCode keycode) { return IsKeyPressedImpl(keycode); }

        static bool IsMouseButtonPressed(const MouseButton button) { return IsMouseButtonPressedImpl(button); }
        static std::pair<float, float> GetMousePosition() { return GetMousePositionImpl(); }
        static float GetMouseX() { return GetMouseXImpl(); }
        static float GetMouseY() { return GetMouseYImpl(); }
    protected:
        static bool IsKeyPressedImpl(KeyCode keycode);

        static bool IsMouseButtonPressedImpl(MouseButton button);
        static std::pair<float, float> GetMousePositionImpl();
        static float GetMouseXImpl();
        static float GetMouseYImpl();
    private:
        static Input* s_Instance;
    };
}
#pragma once

#include "KeyCodes.h"

class CWindow;

class SInput {
public:
    static void Init(CWindow* window);

    static bool IsKeyPressed(const EKeyCode keycode) { return IsKeyPressedImpl(keycode); }

    static bool IsMouseButtonPressed(const EMouseButton button) {
        return IsMouseButtonPressedImpl(button);
    }
    static std::pair<float, float> GetMousePosition() { return GetMousePositionImpl(); }
    static float GetMouseX() { return GetMouseXImpl(); }
    static float GetMouseY() { return GetMouseYImpl(); }

protected:
    static bool IsKeyPressedImpl(EKeyCode keycode);

    static bool IsMouseButtonPressedImpl(EMouseButton button);
    static std::pair<float, float> GetMousePositionImpl();
    static float GetMouseXImpl();
    static float GetMouseYImpl();

private:
    static SInput* s_Instance;
    CWindow* m_Window;
};
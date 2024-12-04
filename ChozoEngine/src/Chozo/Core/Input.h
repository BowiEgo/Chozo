#pragma once

#include "Chozo/Core/Base.h"
#include "KeyCodes.h"

namespace Chozo {

    class Input
    {
    public:
        // inline static bool IsKeyPressed(int keycode) { return s_Instance->IsKeyPressedImpl(keycode); }
        inline static bool IsKeyPressed(KeyCode keycode) { return s_Instance->IsKeyPressedImpl(keycode); }

        // inline static bool IsMouseButtonPressed(int button) { return s_Instance->IsMouseButtonPressedImpl(button); }
        inline static bool IsMouseButtonPressed(MouseButton button) { return s_Instance->IsMouseButtonPressedImpl(button); }
        inline static std::pair<float, float> GetMousePosition() { return s_Instance->GetMousePositionImpl(); }
        inline static float GetMouseX() { return s_Instance->GetMouseXImpl(); }
        inline static float GetMouseY() { return s_Instance->GetMouseYImpl(); }
    protected:
        // bool IsKeyPressedImpl(int keycode);
        bool IsKeyPressedImpl(KeyCode keycode);

        // bool IsMouseButtonPressedImpl(int button);
        bool IsMouseButtonPressedImpl(MouseButton button);
        std::pair<float, float> GetMousePositionImpl();
        float GetMouseXImpl();
        float GetMouseYImpl();
    private:
        static Input* s_Instance;
    };
}
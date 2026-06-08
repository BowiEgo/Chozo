#pragma once

#include <Core/Event/InputImpl.hpp>
#include <Core/Event/KeyCodes.hpp>
#include <Core/Memory/Memory.hpp>

namespace CZ {

class Input {
    static InputImpl* s_Impl;

public:
    static void Init(InputImpl* impl) { s_Impl = impl; }
    static InputImpl* GetImpl() { return s_Impl; }
    static bool IsKeyPressed(KeyCode keycode) { return s_Impl->IsKeyPressed(keycode); }
    static bool IsMouseButtonPressed(MouseButton button) {
        return s_Impl->IsMouseButtonPressed(button);
    }
    static std::pair<float, float> GetMousePosition() { return s_Impl->GetMousePosition(); }
    static float GetMouseX() { return s_Impl->GetMouseX(); }
    static float GetMouseY() { return s_Impl->GetMouseY(); }
};

} // namespace CZ
#pragma once

#include "InputImpl.h"
#include "KeyCodes.h"

#include "CoreExport.h"
#include "CoreTypes.h"

class CORE_API SInput {
    static IInputImpl* s_Impl;

public:
    static void Init(IInputImpl* impl) { s_Impl = impl; }
    static IInputImpl* GetImpl() { return s_Impl; }
    static bool IsKeyPressed(EKeyCode keycode) { return s_Impl->IsKeyPressed(keycode); }
    static bool IsMouseButtonPressed(EMouseButton button) {
        return s_Impl->IsMouseButtonPressed(button);
    }
    static std::pair<float, float> GetMousePosition() { return s_Impl->GetMousePosition(); }
    static float GetMouseX() { return s_Impl->GetMouseX(); }
    static float GetMouseY() { return s_Impl->GetMouseY(); }
};
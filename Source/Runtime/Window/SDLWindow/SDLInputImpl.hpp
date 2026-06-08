#pragma once

#include <Core/Event/InputImpl.hpp>

#include "SDLKeyMap.hpp"

#include <SDL3/SDL.h>
#include <SDL3/SDL_mouse.h>

namespace CZ {

class SDLInputImpl : public InputImpl {
public:
    SDLInputImpl() {}

    virtual bool IsKeyPressed(KeyCode keycode) override {
        SDL_Scancode scancode = EngineKeyToSDLScancode(keycode);
        if (scancode == SDL_SCANCODE_UNKNOWN) {
            return false;
        }
        const bool* state = SDL_GetKeyboardState(nullptr);
        return state[scancode];
    }

    virtual bool IsMouseButtonPressed(MouseButton button) override {
        Uint32 buttons = SDL_GetMouseState(NULL, NULL);
        switch (button) {
            case MouseButton::Left: return (buttons & SDL_BUTTON_LMASK) != 0;
            case MouseButton::Right: return (buttons & SDL_BUTTON_RMASK) != 0;
            case MouseButton::Middle: return (buttons & SDL_BUTTON_MMASK) != 0;
            // case MouseButton::Side1: return (buttons & SDL_BUTTON_X1MASK) != 0;
            // case MouseButton::Side2: return (buttons & SDL_BUTTON_X2MASK) != 0;
            default: return false;
        }
    }

    virtual std::pair<float, float> GetMousePosition() override {
        float x, y;
        SDL_GetMouseState(&x, &y);
        return { x, y };
    }

    virtual float GetMouseX() override {
        auto [x, y] = GetMousePosition();
        return x;
    }

    virtual float GetMouseY() override {
        auto [x, y] = GetMousePosition();
        return y;
    }
};

} // namespace CZ
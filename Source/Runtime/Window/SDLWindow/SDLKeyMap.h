#pragma once

#include <Core/Event/KeyCodes.h>

#include <SDL3/SDL.h>

namespace CZ {

// EMouseButton MapSDLMouseButtonToEngine(Uint8 sdlButton) {
//     switch (sdlButton) {
//         case SDL_BUTTON_LEFT: return EMouseButton::Left;
//         case SDL_BUTTON_RIGHT: return EMouseButton::Right;
//         case SDL_BUTTON_MIDDLE: return EMouseButton::Middle;
//         // case SDL_BUTTON_X1: return EMouseButton::Side1;
//         // case SDL_BUTTON_X2: return EMouseButton::Side2;
//         default: return EMouseButton::Left;
//     }
// }

KeyCode SDLScancodeToEngineKey(SDL_Scancode scancode) {
    switch (scancode) {
        // 字母
        case SDL_SCANCODE_A: return KeyCode::A;
        case SDL_SCANCODE_B: return KeyCode::B;
        case SDL_SCANCODE_C: return KeyCode::C;
        case SDL_SCANCODE_D: return KeyCode::D;
        case SDL_SCANCODE_E: return KeyCode::E;
        case SDL_SCANCODE_F: return KeyCode::F;
        case SDL_SCANCODE_G: return KeyCode::G;
        case SDL_SCANCODE_H: return KeyCode::H;
        case SDL_SCANCODE_I: return KeyCode::I;
        case SDL_SCANCODE_J: return KeyCode::J;
        case SDL_SCANCODE_K: return KeyCode::K;
        case SDL_SCANCODE_L: return KeyCode::L;
        case SDL_SCANCODE_M: return KeyCode::M;
        case SDL_SCANCODE_N: return KeyCode::N;
        case SDL_SCANCODE_O: return KeyCode::O;
        case SDL_SCANCODE_P: return KeyCode::P;
        case SDL_SCANCODE_Q: return KeyCode::Q;
        case SDL_SCANCODE_R: return KeyCode::R;
        case SDL_SCANCODE_S: return KeyCode::S;
        case SDL_SCANCODE_T: return KeyCode::T;
        case SDL_SCANCODE_U: return KeyCode::U;
        case SDL_SCANCODE_V: return KeyCode::V;
        case SDL_SCANCODE_W: return KeyCode::W;
        case SDL_SCANCODE_X: return KeyCode::X;
        case SDL_SCANCODE_Y: return KeyCode::Y;
        case SDL_SCANCODE_Z: return KeyCode::Z;
        // 数字
        case SDL_SCANCODE_1: return KeyCode::D1;
        case SDL_SCANCODE_2: return KeyCode::D2;
        case SDL_SCANCODE_3: return KeyCode::D3;
        case SDL_SCANCODE_4: return KeyCode::D4;
        case SDL_SCANCODE_5: return KeyCode::D5;
        case SDL_SCANCODE_6: return KeyCode::D6;
        case SDL_SCANCODE_7: return KeyCode::D7;
        case SDL_SCANCODE_8: return KeyCode::D8;
        case SDL_SCANCODE_9: return KeyCode::D9;
        case SDL_SCANCODE_0: return KeyCode::D0;
        // 功能键
        case SDL_SCANCODE_ESCAPE: return KeyCode::Escape;
        case SDL_SCANCODE_RETURN: return KeyCode::Enter;
        case SDL_SCANCODE_TAB: return KeyCode::Tab;
        case SDL_SCANCODE_BACKSPACE: return KeyCode::Backspace;
        case SDL_SCANCODE_INSERT: return KeyCode::Insert;
        case SDL_SCANCODE_DELETE: return KeyCode::Delete;
        case SDL_SCANCODE_RIGHT: return KeyCode::Right;
        case SDL_SCANCODE_LEFT: return KeyCode::Left;
        case SDL_SCANCODE_DOWN: return KeyCode::Down;
        case SDL_SCANCODE_UP: return KeyCode::Up;
        case SDL_SCANCODE_PAGEUP: return KeyCode::PageUp;
        case SDL_SCANCODE_PAGEDOWN: return KeyCode::PageDown;
        case SDL_SCANCODE_HOME: return KeyCode::Home;
        case SDL_SCANCODE_END: return KeyCode::End;
        case SDL_SCANCODE_CAPSLOCK: return KeyCode::CapsLock;
        case SDL_SCANCODE_SCROLLLOCK: return KeyCode::ScrollLock;
        case SDL_SCANCODE_NUMLOCKCLEAR: return KeyCode::NumLock;
        case SDL_SCANCODE_PRINTSCREEN: return KeyCode::PrintScreen;
        case SDL_SCANCODE_PAUSE: return KeyCode::Pause;
        // F键
        case SDL_SCANCODE_F1: return KeyCode::F1;
        case SDL_SCANCODE_F2: return KeyCode::F2;
        case SDL_SCANCODE_F3: return KeyCode::F3;
        case SDL_SCANCODE_F4: return KeyCode::F4;
        case SDL_SCANCODE_F5: return KeyCode::F5;
        case SDL_SCANCODE_F6: return KeyCode::F6;
        case SDL_SCANCODE_F7: return KeyCode::F7;
        case SDL_SCANCODE_F8: return KeyCode::F8;
        case SDL_SCANCODE_F9: return KeyCode::F9;
        case SDL_SCANCODE_F10: return KeyCode::F10;
        case SDL_SCANCODE_F11: return KeyCode::F11;
        case SDL_SCANCODE_F12: return KeyCode::F12;
        case SDL_SCANCODE_F13: return KeyCode::F13;
        case SDL_SCANCODE_F14: return KeyCode::F14;
        case SDL_SCANCODE_F15: return KeyCode::F15;
        case SDL_SCANCODE_F16: return KeyCode::F16;
        case SDL_SCANCODE_F17: return KeyCode::F17;
        case SDL_SCANCODE_F18: return KeyCode::F18;
        case SDL_SCANCODE_F19: return KeyCode::F19;
        case SDL_SCANCODE_F20: return KeyCode::F20;
        case SDL_SCANCODE_F21: return KeyCode::F21;
        case SDL_SCANCODE_F22: return KeyCode::F22;
        case SDL_SCANCODE_F23: return KeyCode::F23;
        case SDL_SCANCODE_F24: return KeyCode::F24;
        // 小键盘
        case SDL_SCANCODE_KP_0: return KeyCode::KP0;
        case SDL_SCANCODE_KP_1: return KeyCode::KP1;
        case SDL_SCANCODE_KP_2: return KeyCode::KP2;
        case SDL_SCANCODE_KP_3: return KeyCode::KP3;
        case SDL_SCANCODE_KP_4: return KeyCode::KP4;
        case SDL_SCANCODE_KP_5: return KeyCode::KP5;
        case SDL_SCANCODE_KP_6: return KeyCode::KP6;
        case SDL_SCANCODE_KP_7: return KeyCode::KP7;
        case SDL_SCANCODE_KP_8: return KeyCode::KP8;
        case SDL_SCANCODE_KP_9: return KeyCode::KP9;
        case SDL_SCANCODE_KP_DECIMAL: return KeyCode::KPDecimal;
        case SDL_SCANCODE_KP_DIVIDE: return KeyCode::KPDivide;
        case SDL_SCANCODE_KP_MULTIPLY: return KeyCode::KPMultiply;
        case SDL_SCANCODE_KP_MINUS: return KeyCode::KPSubtract;
        case SDL_SCANCODE_KP_PLUS: return KeyCode::KPAdd;
        case SDL_SCANCODE_KP_ENTER: return KeyCode::KPEnter;
        case SDL_SCANCODE_KP_EQUALS: return KeyCode::KPEqual;
        // 修饰键
        case SDL_SCANCODE_LSHIFT: return KeyCode::LeftShift;
        case SDL_SCANCODE_RSHIFT: return KeyCode::RightShift;
        case SDL_SCANCODE_LCTRL: return KeyCode::LeftControl;
        case SDL_SCANCODE_RCTRL: return KeyCode::RightControl;
        case SDL_SCANCODE_LALT: return KeyCode::LeftAlt;
        case SDL_SCANCODE_RALT: return KeyCode::RightAlt;
        case SDL_SCANCODE_LGUI: return KeyCode::LeftSuper;
        case SDL_SCANCODE_RGUI: return KeyCode::RightSuper;
        case SDL_SCANCODE_APPLICATION: return KeyCode::Menu;
        default: return KeyCode::Unknown;
    }
}

SDL_Scancode EngineKeyToSDLScancode(KeyCode key) {
    switch (key) {
        case KeyCode::A: return SDL_SCANCODE_A;
        case KeyCode::B: return SDL_SCANCODE_B;
        case KeyCode::C: return SDL_SCANCODE_C;
        case KeyCode::D: return SDL_SCANCODE_D;
        case KeyCode::E: return SDL_SCANCODE_E;
        case KeyCode::F: return SDL_SCANCODE_F;
        case KeyCode::G: return SDL_SCANCODE_G;
        case KeyCode::H: return SDL_SCANCODE_H;
        case KeyCode::I: return SDL_SCANCODE_I;
        case KeyCode::J: return SDL_SCANCODE_J;
        case KeyCode::K: return SDL_SCANCODE_K;
        case KeyCode::L: return SDL_SCANCODE_L;
        case KeyCode::M: return SDL_SCANCODE_M;
        case KeyCode::N: return SDL_SCANCODE_N;
        case KeyCode::O: return SDL_SCANCODE_O;
        case KeyCode::P: return SDL_SCANCODE_P;
        case KeyCode::Q: return SDL_SCANCODE_Q;
        case KeyCode::R: return SDL_SCANCODE_R;
        case KeyCode::S: return SDL_SCANCODE_S;
        case KeyCode::T: return SDL_SCANCODE_T;
        case KeyCode::U: return SDL_SCANCODE_U;
        case KeyCode::V: return SDL_SCANCODE_V;
        case KeyCode::W: return SDL_SCANCODE_W;
        case KeyCode::X: return SDL_SCANCODE_X;
        case KeyCode::Y: return SDL_SCANCODE_Y;
        case KeyCode::Z: return SDL_SCANCODE_Z;
        case KeyCode::D1: return SDL_SCANCODE_1;
        case KeyCode::D2: return SDL_SCANCODE_2;
        case KeyCode::D3: return SDL_SCANCODE_3;
        case KeyCode::D4: return SDL_SCANCODE_4;
        case KeyCode::D5: return SDL_SCANCODE_5;
        case KeyCode::D6: return SDL_SCANCODE_6;
        case KeyCode::D7: return SDL_SCANCODE_7;
        case KeyCode::D8: return SDL_SCANCODE_8;
        case KeyCode::D9: return SDL_SCANCODE_9;
        case KeyCode::D0: return SDL_SCANCODE_0;
        case KeyCode::Space: return SDL_SCANCODE_SPACE;
        case KeyCode::Escape: return SDL_SCANCODE_ESCAPE;
        case KeyCode::Enter: return SDL_SCANCODE_RETURN;
        case KeyCode::Tab: return SDL_SCANCODE_TAB;
        case KeyCode::Backspace: return SDL_SCANCODE_BACKSPACE;
        case KeyCode::Insert: return SDL_SCANCODE_INSERT;
        case KeyCode::Delete: return SDL_SCANCODE_DELETE;
        case KeyCode::Right: return SDL_SCANCODE_RIGHT;
        case KeyCode::Left: return SDL_SCANCODE_LEFT;
        case KeyCode::Down: return SDL_SCANCODE_DOWN;
        case KeyCode::Up: return SDL_SCANCODE_UP;
        case KeyCode::PageUp: return SDL_SCANCODE_PAGEUP;
        case KeyCode::PageDown: return SDL_SCANCODE_PAGEDOWN;
        case KeyCode::Home: return SDL_SCANCODE_HOME;
        case KeyCode::End: return SDL_SCANCODE_END;
        case KeyCode::CapsLock: return SDL_SCANCODE_CAPSLOCK;
        case KeyCode::ScrollLock: return SDL_SCANCODE_SCROLLLOCK;
        case KeyCode::NumLock: return SDL_SCANCODE_NUMLOCKCLEAR;
        case KeyCode::PrintScreen: return SDL_SCANCODE_PRINTSCREEN;
        case KeyCode::Pause: return SDL_SCANCODE_PAUSE;
        case KeyCode::F1: return SDL_SCANCODE_F1;
        case KeyCode::F2: return SDL_SCANCODE_F2;
        case KeyCode::F3: return SDL_SCANCODE_F3;
        case KeyCode::F4: return SDL_SCANCODE_F4;
        case KeyCode::F5: return SDL_SCANCODE_F5;
        case KeyCode::F6: return SDL_SCANCODE_F6;
        case KeyCode::F7: return SDL_SCANCODE_F7;
        case KeyCode::F8: return SDL_SCANCODE_F8;
        case KeyCode::F9: return SDL_SCANCODE_F9;
        case KeyCode::F10: return SDL_SCANCODE_F10;
        case KeyCode::F11: return SDL_SCANCODE_F11;
        case KeyCode::F12: return SDL_SCANCODE_F12;
        case KeyCode::F13: return SDL_SCANCODE_F13;
        case KeyCode::F14: return SDL_SCANCODE_F14;
        case KeyCode::F15: return SDL_SCANCODE_F15;
        case KeyCode::F16: return SDL_SCANCODE_F16;
        case KeyCode::F17: return SDL_SCANCODE_F17;
        case KeyCode::F18: return SDL_SCANCODE_F18;
        case KeyCode::F19: return SDL_SCANCODE_F19;
        case KeyCode::F20: return SDL_SCANCODE_F20;
        case KeyCode::F21: return SDL_SCANCODE_F21;
        case KeyCode::F22: return SDL_SCANCODE_F22;
        case KeyCode::F23: return SDL_SCANCODE_F23;
        case KeyCode::F24: return SDL_SCANCODE_F24;
        case KeyCode::KP0: return SDL_SCANCODE_KP_0;
        case KeyCode::KP1: return SDL_SCANCODE_KP_1;
        case KeyCode::KP2: return SDL_SCANCODE_KP_2;
        case KeyCode::KP3: return SDL_SCANCODE_KP_3;
        case KeyCode::KP4: return SDL_SCANCODE_KP_4;
        case KeyCode::KP5: return SDL_SCANCODE_KP_5;
        case KeyCode::KP6: return SDL_SCANCODE_KP_6;
        case KeyCode::KP7: return SDL_SCANCODE_KP_7;
        case KeyCode::KP8: return SDL_SCANCODE_KP_8;
        case KeyCode::KP9: return SDL_SCANCODE_KP_9;
        case KeyCode::KPDecimal: return SDL_SCANCODE_KP_DECIMAL;
        case KeyCode::KPDivide: return SDL_SCANCODE_KP_DIVIDE;
        case KeyCode::KPMultiply: return SDL_SCANCODE_KP_MULTIPLY;
        case KeyCode::KPSubtract: return SDL_SCANCODE_KP_MINUS;
        case KeyCode::KPAdd: return SDL_SCANCODE_KP_PLUS;
        case KeyCode::KPEnter: return SDL_SCANCODE_KP_ENTER;
        case KeyCode::KPEqual: return SDL_SCANCODE_KP_EQUALS;
        case KeyCode::LeftShift: return SDL_SCANCODE_LSHIFT;
        case KeyCode::RightShift: return SDL_SCANCODE_RSHIFT;
        case KeyCode::LeftControl: return SDL_SCANCODE_LCTRL;
        case KeyCode::RightControl: return SDL_SCANCODE_RCTRL;
        case KeyCode::LeftAlt: return SDL_SCANCODE_LALT;
        case KeyCode::RightAlt: return SDL_SCANCODE_RALT;
        case KeyCode::LeftSuper: return SDL_SCANCODE_LGUI;
        case KeyCode::RightSuper: return SDL_SCANCODE_RGUI;
        case KeyCode::Menu: return SDL_SCANCODE_APPLICATION;
        default: return SDL_SCANCODE_UNKNOWN;
    }
}

} // namespace CZ

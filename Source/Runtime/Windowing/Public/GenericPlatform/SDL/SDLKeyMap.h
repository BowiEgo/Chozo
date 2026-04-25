#pragma once

#include <SDL3/SDL.h>

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

EKeyCode SDLScancodeToEngineKey(SDL_Scancode scancode) {
    switch (scancode) {
        // 字母
        case SDL_SCANCODE_A: return EKeyCode::A;
        case SDL_SCANCODE_B: return EKeyCode::B;
        case SDL_SCANCODE_C: return EKeyCode::C;
        case SDL_SCANCODE_D: return EKeyCode::D;
        case SDL_SCANCODE_E: return EKeyCode::E;
        case SDL_SCANCODE_F: return EKeyCode::F;
        case SDL_SCANCODE_G: return EKeyCode::G;
        case SDL_SCANCODE_H: return EKeyCode::H;
        case SDL_SCANCODE_I: return EKeyCode::I;
        case SDL_SCANCODE_J: return EKeyCode::J;
        case SDL_SCANCODE_K: return EKeyCode::K;
        case SDL_SCANCODE_L: return EKeyCode::L;
        case SDL_SCANCODE_M: return EKeyCode::M;
        case SDL_SCANCODE_N: return EKeyCode::N;
        case SDL_SCANCODE_O: return EKeyCode::O;
        case SDL_SCANCODE_P: return EKeyCode::P;
        case SDL_SCANCODE_Q: return EKeyCode::Q;
        case SDL_SCANCODE_R: return EKeyCode::R;
        case SDL_SCANCODE_S: return EKeyCode::S;
        case SDL_SCANCODE_T: return EKeyCode::T;
        case SDL_SCANCODE_U: return EKeyCode::U;
        case SDL_SCANCODE_V: return EKeyCode::V;
        case SDL_SCANCODE_W: return EKeyCode::W;
        case SDL_SCANCODE_X: return EKeyCode::X;
        case SDL_SCANCODE_Y: return EKeyCode::Y;
        case SDL_SCANCODE_Z: return EKeyCode::Z;
        // 数字
        case SDL_SCANCODE_1: return EKeyCode::D1;
        case SDL_SCANCODE_2: return EKeyCode::D2;
        case SDL_SCANCODE_3: return EKeyCode::D3;
        case SDL_SCANCODE_4: return EKeyCode::D4;
        case SDL_SCANCODE_5: return EKeyCode::D5;
        case SDL_SCANCODE_6: return EKeyCode::D6;
        case SDL_SCANCODE_7: return EKeyCode::D7;
        case SDL_SCANCODE_8: return EKeyCode::D8;
        case SDL_SCANCODE_9: return EKeyCode::D9;
        case SDL_SCANCODE_0: return EKeyCode::D0;
        // 功能键
        case SDL_SCANCODE_ESCAPE: return EKeyCode::Escape;
        case SDL_SCANCODE_RETURN: return EKeyCode::Enter;
        case SDL_SCANCODE_TAB: return EKeyCode::Tab;
        case SDL_SCANCODE_BACKSPACE: return EKeyCode::Backspace;
        case SDL_SCANCODE_INSERT: return EKeyCode::Insert;
        case SDL_SCANCODE_DELETE: return EKeyCode::Delete;
        case SDL_SCANCODE_RIGHT: return EKeyCode::Right;
        case SDL_SCANCODE_LEFT: return EKeyCode::Left;
        case SDL_SCANCODE_DOWN: return EKeyCode::Down;
        case SDL_SCANCODE_UP: return EKeyCode::Up;
        case SDL_SCANCODE_PAGEUP: return EKeyCode::PageUp;
        case SDL_SCANCODE_PAGEDOWN: return EKeyCode::PageDown;
        case SDL_SCANCODE_HOME: return EKeyCode::Home;
        case SDL_SCANCODE_END: return EKeyCode::End;
        case SDL_SCANCODE_CAPSLOCK: return EKeyCode::CapsLock;
        case SDL_SCANCODE_SCROLLLOCK: return EKeyCode::ScrollLock;
        case SDL_SCANCODE_NUMLOCKCLEAR: return EKeyCode::NumLock;
        case SDL_SCANCODE_PRINTSCREEN: return EKeyCode::PrintScreen;
        case SDL_SCANCODE_PAUSE: return EKeyCode::Pause;
        // F键
        case SDL_SCANCODE_F1: return EKeyCode::F1;
        case SDL_SCANCODE_F2: return EKeyCode::F2;
        case SDL_SCANCODE_F3: return EKeyCode::F3;
        case SDL_SCANCODE_F4: return EKeyCode::F4;
        case SDL_SCANCODE_F5: return EKeyCode::F5;
        case SDL_SCANCODE_F6: return EKeyCode::F6;
        case SDL_SCANCODE_F7: return EKeyCode::F7;
        case SDL_SCANCODE_F8: return EKeyCode::F8;
        case SDL_SCANCODE_F9: return EKeyCode::F9;
        case SDL_SCANCODE_F10: return EKeyCode::F10;
        case SDL_SCANCODE_F11: return EKeyCode::F11;
        case SDL_SCANCODE_F12: return EKeyCode::F12;
        case SDL_SCANCODE_F13: return EKeyCode::F13;
        case SDL_SCANCODE_F14: return EKeyCode::F14;
        case SDL_SCANCODE_F15: return EKeyCode::F15;
        case SDL_SCANCODE_F16: return EKeyCode::F16;
        case SDL_SCANCODE_F17: return EKeyCode::F17;
        case SDL_SCANCODE_F18: return EKeyCode::F18;
        case SDL_SCANCODE_F19: return EKeyCode::F19;
        case SDL_SCANCODE_F20: return EKeyCode::F20;
        case SDL_SCANCODE_F21: return EKeyCode::F21;
        case SDL_SCANCODE_F22: return EKeyCode::F22;
        case SDL_SCANCODE_F23: return EKeyCode::F23;
        case SDL_SCANCODE_F24: return EKeyCode::F24;
        // 小键盘
        case SDL_SCANCODE_KP_0: return EKeyCode::KP0;
        case SDL_SCANCODE_KP_1: return EKeyCode::KP1;
        case SDL_SCANCODE_KP_2: return EKeyCode::KP2;
        case SDL_SCANCODE_KP_3: return EKeyCode::KP3;
        case SDL_SCANCODE_KP_4: return EKeyCode::KP4;
        case SDL_SCANCODE_KP_5: return EKeyCode::KP5;
        case SDL_SCANCODE_KP_6: return EKeyCode::KP6;
        case SDL_SCANCODE_KP_7: return EKeyCode::KP7;
        case SDL_SCANCODE_KP_8: return EKeyCode::KP8;
        case SDL_SCANCODE_KP_9: return EKeyCode::KP9;
        case SDL_SCANCODE_KP_DECIMAL: return EKeyCode::KPDecimal;
        case SDL_SCANCODE_KP_DIVIDE: return EKeyCode::KPDivide;
        case SDL_SCANCODE_KP_MULTIPLY: return EKeyCode::KPMultiply;
        case SDL_SCANCODE_KP_MINUS: return EKeyCode::KPSubtract;
        case SDL_SCANCODE_KP_PLUS: return EKeyCode::KPAdd;
        case SDL_SCANCODE_KP_ENTER: return EKeyCode::KPEnter;
        case SDL_SCANCODE_KP_EQUALS: return EKeyCode::KPEqual;
        // 修饰键
        case SDL_SCANCODE_LSHIFT: return EKeyCode::LeftShift;
        case SDL_SCANCODE_RSHIFT: return EKeyCode::RightShift;
        case SDL_SCANCODE_LCTRL: return EKeyCode::LeftControl;
        case SDL_SCANCODE_RCTRL: return EKeyCode::RightControl;
        case SDL_SCANCODE_LALT: return EKeyCode::LeftAlt;
        case SDL_SCANCODE_RALT: return EKeyCode::RightAlt;
        case SDL_SCANCODE_LGUI: return EKeyCode::LeftSuper;
        case SDL_SCANCODE_RGUI: return EKeyCode::RightSuper;
        case SDL_SCANCODE_APPLICATION: return EKeyCode::Menu;
        default: return EKeyCode::Unknown;
    }
}

SDL_Scancode EngineKeyToSDLScancode(EKeyCode key) {
    switch (key) {
        case EKeyCode::A: return SDL_SCANCODE_A;
        case EKeyCode::B: return SDL_SCANCODE_B;
        case EKeyCode::C: return SDL_SCANCODE_C;
        case EKeyCode::D: return SDL_SCANCODE_D;
        case EKeyCode::E: return SDL_SCANCODE_E;
        case EKeyCode::F: return SDL_SCANCODE_F;
        case EKeyCode::G: return SDL_SCANCODE_G;
        case EKeyCode::H: return SDL_SCANCODE_H;
        case EKeyCode::I: return SDL_SCANCODE_I;
        case EKeyCode::J: return SDL_SCANCODE_J;
        case EKeyCode::K: return SDL_SCANCODE_K;
        case EKeyCode::L: return SDL_SCANCODE_L;
        case EKeyCode::M: return SDL_SCANCODE_M;
        case EKeyCode::N: return SDL_SCANCODE_N;
        case EKeyCode::O: return SDL_SCANCODE_O;
        case EKeyCode::P: return SDL_SCANCODE_P;
        case EKeyCode::Q: return SDL_SCANCODE_Q;
        case EKeyCode::R: return SDL_SCANCODE_R;
        case EKeyCode::S: return SDL_SCANCODE_S;
        case EKeyCode::T: return SDL_SCANCODE_T;
        case EKeyCode::U: return SDL_SCANCODE_U;
        case EKeyCode::V: return SDL_SCANCODE_V;
        case EKeyCode::W: return SDL_SCANCODE_W;
        case EKeyCode::X: return SDL_SCANCODE_X;
        case EKeyCode::Y: return SDL_SCANCODE_Y;
        case EKeyCode::Z: return SDL_SCANCODE_Z;
        case EKeyCode::D1: return SDL_SCANCODE_1;
        case EKeyCode::D2: return SDL_SCANCODE_2;
        case EKeyCode::D3: return SDL_SCANCODE_3;
        case EKeyCode::D4: return SDL_SCANCODE_4;
        case EKeyCode::D5: return SDL_SCANCODE_5;
        case EKeyCode::D6: return SDL_SCANCODE_6;
        case EKeyCode::D7: return SDL_SCANCODE_7;
        case EKeyCode::D8: return SDL_SCANCODE_8;
        case EKeyCode::D9: return SDL_SCANCODE_9;
        case EKeyCode::D0: return SDL_SCANCODE_0;
        case EKeyCode::Space: return SDL_SCANCODE_SPACE;
        case EKeyCode::Escape: return SDL_SCANCODE_ESCAPE;
        case EKeyCode::Enter: return SDL_SCANCODE_RETURN;
        case EKeyCode::Tab: return SDL_SCANCODE_TAB;
        case EKeyCode::Backspace: return SDL_SCANCODE_BACKSPACE;
        case EKeyCode::Insert: return SDL_SCANCODE_INSERT;
        case EKeyCode::Delete: return SDL_SCANCODE_DELETE;
        case EKeyCode::Right: return SDL_SCANCODE_RIGHT;
        case EKeyCode::Left: return SDL_SCANCODE_LEFT;
        case EKeyCode::Down: return SDL_SCANCODE_DOWN;
        case EKeyCode::Up: return SDL_SCANCODE_UP;
        case EKeyCode::PageUp: return SDL_SCANCODE_PAGEUP;
        case EKeyCode::PageDown: return SDL_SCANCODE_PAGEDOWN;
        case EKeyCode::Home: return SDL_SCANCODE_HOME;
        case EKeyCode::End: return SDL_SCANCODE_END;
        case EKeyCode::CapsLock: return SDL_SCANCODE_CAPSLOCK;
        case EKeyCode::ScrollLock: return SDL_SCANCODE_SCROLLLOCK;
        case EKeyCode::NumLock: return SDL_SCANCODE_NUMLOCKCLEAR;
        case EKeyCode::PrintScreen: return SDL_SCANCODE_PRINTSCREEN;
        case EKeyCode::Pause: return SDL_SCANCODE_PAUSE;
        case EKeyCode::F1: return SDL_SCANCODE_F1;
        case EKeyCode::F2: return SDL_SCANCODE_F2;
        case EKeyCode::F3: return SDL_SCANCODE_F3;
        case EKeyCode::F4: return SDL_SCANCODE_F4;
        case EKeyCode::F5: return SDL_SCANCODE_F5;
        case EKeyCode::F6: return SDL_SCANCODE_F6;
        case EKeyCode::F7: return SDL_SCANCODE_F7;
        case EKeyCode::F8: return SDL_SCANCODE_F8;
        case EKeyCode::F9: return SDL_SCANCODE_F9;
        case EKeyCode::F10: return SDL_SCANCODE_F10;
        case EKeyCode::F11: return SDL_SCANCODE_F11;
        case EKeyCode::F12: return SDL_SCANCODE_F12;
        case EKeyCode::F13: return SDL_SCANCODE_F13;
        case EKeyCode::F14: return SDL_SCANCODE_F14;
        case EKeyCode::F15: return SDL_SCANCODE_F15;
        case EKeyCode::F16: return SDL_SCANCODE_F16;
        case EKeyCode::F17: return SDL_SCANCODE_F17;
        case EKeyCode::F18: return SDL_SCANCODE_F18;
        case EKeyCode::F19: return SDL_SCANCODE_F19;
        case EKeyCode::F20: return SDL_SCANCODE_F20;
        case EKeyCode::F21: return SDL_SCANCODE_F21;
        case EKeyCode::F22: return SDL_SCANCODE_F22;
        case EKeyCode::F23: return SDL_SCANCODE_F23;
        case EKeyCode::F24: return SDL_SCANCODE_F24;
        case EKeyCode::KP0: return SDL_SCANCODE_KP_0;
        case EKeyCode::KP1: return SDL_SCANCODE_KP_1;
        case EKeyCode::KP2: return SDL_SCANCODE_KP_2;
        case EKeyCode::KP3: return SDL_SCANCODE_KP_3;
        case EKeyCode::KP4: return SDL_SCANCODE_KP_4;
        case EKeyCode::KP5: return SDL_SCANCODE_KP_5;
        case EKeyCode::KP6: return SDL_SCANCODE_KP_6;
        case EKeyCode::KP7: return SDL_SCANCODE_KP_7;
        case EKeyCode::KP8: return SDL_SCANCODE_KP_8;
        case EKeyCode::KP9: return SDL_SCANCODE_KP_9;
        case EKeyCode::KPDecimal: return SDL_SCANCODE_KP_DECIMAL;
        case EKeyCode::KPDivide: return SDL_SCANCODE_KP_DIVIDE;
        case EKeyCode::KPMultiply: return SDL_SCANCODE_KP_MULTIPLY;
        case EKeyCode::KPSubtract: return SDL_SCANCODE_KP_MINUS;
        case EKeyCode::KPAdd: return SDL_SCANCODE_KP_PLUS;
        case EKeyCode::KPEnter: return SDL_SCANCODE_KP_ENTER;
        case EKeyCode::KPEqual: return SDL_SCANCODE_KP_EQUALS;
        case EKeyCode::LeftShift: return SDL_SCANCODE_LSHIFT;
        case EKeyCode::RightShift: return SDL_SCANCODE_RSHIFT;
        case EKeyCode::LeftControl: return SDL_SCANCODE_LCTRL;
        case EKeyCode::RightControl: return SDL_SCANCODE_RCTRL;
        case EKeyCode::LeftAlt: return SDL_SCANCODE_LALT;
        case EKeyCode::RightAlt: return SDL_SCANCODE_RALT;
        case EKeyCode::LeftSuper: return SDL_SCANCODE_LGUI;
        case EKeyCode::RightSuper: return SDL_SCANCODE_RGUI;
        case EKeyCode::Menu: return SDL_SCANCODE_APPLICATION;
        default: return SDL_SCANCODE_UNKNOWN;
    }
}

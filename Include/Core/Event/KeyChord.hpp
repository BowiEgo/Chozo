#pragma once
#include <Core/Event/KeyCodes.hpp>

#include <cstdio>
#include <string>

namespace CZ {

/// A keyboard chord: one key plus the modifiers that must be held with it.
///
/// The engine's KeyCode mixes two ranges -- letters are ASCII (Z is 90) while the modifiers use SDL
/// scancode-range values (LeftSuper is 343) -- and the platform decides whether the "primary"
/// modifier is Command or Control. Both facts live here, once, so no call site has to know either.
struct KeyChord {
    KeyCode Key{};
    bool bPrimary = false; ///< Command on macOS, Control elsewhere
    bool bShift   = false;
    bool bAlt     = false;

    /// A plain key with no modifier (named Of because the struct has a Key member).
    static constexpr KeyChord Of(KeyCode key) { return KeyChord{ key }; }

    /// The platform's primary modifier (Command/Control) plus a key -- the usual editor shortcut.
    static constexpr KeyChord Primary(KeyCode key) {
        KeyChord chord{};
        chord.Key      = key;
        chord.bPrimary = true;
        return chord;
    }

    constexpr KeyChord WithShift() const {
        KeyChord copy = *this;
        copy.bShift   = true;
        return copy;
    }
};

/// Display name of a key. Alphanumerics are ASCII in this engine's KeyCode (Z is 90), function keys
/// are spelled out, and anything else falls back to its numeric code rather than showing nothing.
inline const char* KeyName(KeyCode key) {
    switch (key) {
        case KeyCode::F1: return "F1";
        case KeyCode::F2: return "F2";
        case KeyCode::F3: return "F3";
        case KeyCode::F4: return "F4";
        case KeyCode::F5: return "F5";
        case KeyCode::F6: return "F6";
        case KeyCode::F7: return "F7";
        case KeyCode::F8: return "F8";
        case KeyCode::F9: return "F9";
        case KeyCode::F10: return "F10";
        case KeyCode::F11: return "F11";
        case KeyCode::F12: return "F12";
        case KeyCode::Escape: return "Esc";
        default: break;
    }

    static thread_local char s_Buffer[8];
    if (key >= KeyCode::A && key <= KeyCode::Z) {
        s_Buffer[0] = static_cast<char>(key);
        s_Buffer[1] = '\0';
        return s_Buffer;
    }
    std::snprintf(s_Buffer, sizeof(s_Buffer), "Key %d", static_cast<int>(key));
    return s_Buffer;
}

/// Human readable chord, e.g. "Cmd+Shift+Z" on macOS or "Ctrl+Shift+Z" elsewhere.
inline std::string ToString(const KeyChord& chord) {
    std::string text;
#if defined(__APPLE__)
    if (chord.bPrimary) text += "Cmd+";
    if (chord.bAlt) text += "Option+";
#else
    if (chord.bPrimary) text += "Ctrl+";
    if (chord.bAlt) text += "Alt+";
#endif
    if (chord.bShift) text += "Shift+";
    text += KeyName(chord.Key);
    return text;
}

} // namespace CZ

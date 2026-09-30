#pragma once
#include <Core/Event/KeyCodes.hpp>

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

} // namespace CZ

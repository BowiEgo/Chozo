#pragma once
#include <functional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <Core/Event/KeyChord.hpp>
#include <Core/Log/LogMacros.hpp>

namespace CZ {

/// Table of keyboard shortcuts: the chords and their actions are data, edge detection happens once
/// here, and the key state is injected so the whole thing is testable without a window.
///
/// Call sites register a chord and an action; nothing else in the editor keeps track of "was this
/// key down last frame", which is what made the previous per-shortcut booleans easy to forget.
class ShortcutRegistry {
public:
    struct Entry {
        KeyChord Chord;
        std::string Name;
        std::function<void()> Action;
        bool bWasDown = false;
    };

    void Add(KeyChord chord, std::string name, std::function<void()> action) {
        Entry entry{};
        entry.Chord  = chord;
        entry.Name   = std::move(name);
        entry.Action = std::move(action);
        m_Entries.push_back(std::move(entry));
    }

    /// isKeyDown: key state (wrap Input::IsKeyPressed). isBlocked: true while a text field owns the
    /// keyboard, so the shortcut does not fire while the user is typing (it may be null).
    /// Returns the names of the shortcuts that fired, which the editor logs.
    std::vector<std::string_view> Update(const std::function<bool(KeyCode)>& isKeyDown,
                                         const std::function<bool()>& isBlocked = {}) {
        std::vector<std::string_view> fired;
        const bool bBlocked = isBlocked && isBlocked();

        for (Entry& entry : m_Entries) {
            const bool bDown = isKeyDown(entry.Chord.Key) && ModifiersHeld(entry.Chord, isKeyDown);
            if (bDown && !entry.bWasDown && !bBlocked) {
                if (entry.Action) {
                    entry.Action();
                }
                fired.emplace_back(entry.Name);
            }
            entry.bWasDown = bDown;
        }
        return fired;
    }

    const std::vector<Entry>& Entries() const { return m_Entries; }

private:
    static bool ModifiersHeld(const KeyChord& chord,
                              const std::function<bool(KeyCode)>& isKeyDown) {
        const auto any = [&isKeyDown](KeyCode a, KeyCode b) {
            return isKeyDown(a) || isKeyDown(b);
        };
        // The primary modifier is Command on macOS and Control everywhere else: the single place
        // the platform difference appears.
#if defined(__APPLE__)
        const bool bPrimaryHeld = any(KeyCode::LeftSuper, KeyCode::RightSuper);
        const bool bCtrlHeld    = any(KeyCode::LeftControl, KeyCode::RightControl);
#else
        const bool bPrimaryHeld = any(KeyCode::LeftControl, KeyCode::RightControl);
        const bool bCtrlHeld    = bPrimaryHeld;
#endif
        const bool bShiftHeld = any(KeyCode::LeftShift, KeyCode::RightShift);
        const bool bAltHeld   = any(KeyCode::LeftAlt, KeyCode::RightAlt);

        if (chord.bPrimary != bPrimaryHeld) {
            return false;
        }
        if (chord.bShift != bShiftHeld) {
            return false;
        }
        if (chord.bAlt != bAltHeld) {
            return false;
        }
        // On non-Apple platforms primary and control are the same key, so nothing extra to check.
        (void)bCtrlHeld;
        return true;
    }

    std::vector<Entry> m_Entries;
};

} // namespace CZ

// Shortcut chords and the registry that turns them into actions. The key state and the blocked
// predicate are injected, so the whole thing is exercised without a window or an input device.
#include <Core/Event/ShortcutRegistry.hpp>
#include <algorithm>
#include <functional>
#include <vector>

#include <doctest/doctest.h>

using namespace CZ;

namespace {
/// Fake key state: only the keys a test marks as down report true.
struct FakeKeys {
    std::vector<KeyCode> Down;

    std::function<bool(KeyCode)> AsPredicate() const {
        return [keys = Down](KeyCode key) {
            return std::find(keys.begin(), keys.end(), key) != keys.end();
        };
    }
};

constexpr KeyCode kPrimary =
#if defined(__APPLE__)
    KeyCode::LeftSuper;
#else
    KeyCode::LeftControl;
#endif
} // namespace

TEST_CASE("A shortcut fires once per press, not every frame") {
    ShortcutRegistry registry;
    int fired = 0;
    registry.Add(KeyChord::Of(KeyCode::F5), "Wireframe", [&fired]() { ++fired; });

    const FakeKeys down{ { KeyCode::F5 } };
    registry.Update(down.AsPredicate()); // press
    registry.Update(down.AsPredicate()); // still held
    registry.Update(down.AsPredicate()); // still held

    CHECK(fired == 1);
    registry.Update(FakeKeys{}.AsPredicate()); // release
    registry.Update(down.AsPredicate());       // press again
    CHECK(fired == 2);
}

TEST_CASE("A chord only fires when its modifiers match") {
    ShortcutRegistry registry;
    int plain = 0, withShift = 0;
    registry.Add(KeyChord::Of(KeyCode::Z), "Plain", [&plain]() { ++plain; });
    registry.Add(KeyChord::Of(KeyCode::Z).WithShift(), "Shifted", [&withShift]() { ++withShift; });

    registry.Update(FakeKeys{ { kPrimary, KeyCode::Z } }.AsPredicate()); // Z + primary
    CHECK(plain == 0);                                                   // needs no modifier
    CHECK(withShift == 0);                                               // needs Shift

    registry.Update(FakeKeys{}.AsPredicate());                 // release everything
    registry.Update(FakeKeys{ { KeyCode::Z } }.AsPredicate()); // Z alone
    CHECK(plain == 1);
    CHECK(withShift == 0);

    registry.Update(FakeKeys{}.AsPredicate());
    registry.Update(FakeKeys{ { KeyCode::Z, KeyCode::LeftShift } }.AsPredicate());
    CHECK(withShift == 1);
}

TEST_CASE("The primary modifier is Command on macOS and Control elsewhere") {
    ShortcutRegistry registry;
    int fired = 0;
    registry.Add(KeyChord::Primary(KeyCode::Z), "Undo", [&fired]() { ++fired; });

    registry.Update(FakeKeys{ { kPrimary, KeyCode::Z } }.AsPredicate());
    CHECK(fired == 1);
}

TEST_CASE("The blocked predicate suppresses the shortcut, and releasing re-arms it") {
    ShortcutRegistry registry;
    int fired = 0;
    registry.Add(KeyChord::Of(KeyCode::F9), "Overlay", [&fired]() { ++fired; });

    bool bBlocked = true;
    const FakeKeys down{ { KeyCode::F9 } };
    registry.Update(down.AsPredicate(), [&bBlocked]() { return bBlocked; });
    CHECK(fired == 0); // typing keeps the shortcut

    registry.Update(down.AsPredicate(), [&bBlocked]() { return bBlocked; });
    CHECK(fired == 0);

    bBlocked = false;
    registry.Update(FakeKeys{}.AsPredicate(), [&bBlocked]() { return bBlocked; });
    registry.Update(down.AsPredicate(), [&bBlocked]() { return bBlocked; });
    CHECK(fired == 1);
}

TEST_CASE("Entries expose the registered names for the help panel") {
    ShortcutRegistry registry;
    registry.Add(KeyChord::Primary(KeyCode::Z), "Undo", []() {});
    registry.Add(KeyChord::Of(KeyCode::F5), "Toggle wireframe", []() {});

    REQUIRE(registry.Entries().size() == 2);
    CHECK(registry.Entries()[0].Name == "Undo");
    CHECK(registry.Entries()[1].Name == "Toggle wireframe");
}

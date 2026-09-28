#include <string>

#include <Core/Event/AppEvent.hpp>
#include <Core/Event/Event.hpp>
#include <Core/Event/KeyEvent.hpp>
#include <Core/Event/MouseEvent.hpp>

#include <doctest/doctest.h>

using namespace CZ;

TEST_SUITE("Event") {

    TEST_CASE("Static type identifies the event") {
        KeyPressedEvent event(CZ_KEY(A), 1);

        CHECK(event.GetEventType() == EventType::KeyPressed);
        CHECK(std::string(event.GetName()) == "KeyPressed");
        CHECK_FALSE(event.isHandled());
    }

    TEST_CASE("Dispatcher routes to the matching handler") {
        KeyPressedEvent event(CZ_KEY(A), 1);
        EventDispatcher dispatcher(event);

        int calls             = 0;
        const bool dispatched = dispatcher.Dispatch<KeyPressedEvent>([&](KeyPressedEvent& e) {
            calls++;
            CHECK(e.GetKeyCode() == CZ_KEY(A));
            return false;
        });

        CHECK(dispatched);
        CHECK_EQ(calls, 1);
    }

    TEST_CASE("Dispatcher ignores other event types") {
        KeyPressedEvent event(CZ_KEY(A), 1);
        EventDispatcher dispatcher(event);

        int calls             = 0;
        const bool dispatched = dispatcher.Dispatch<MouseMovedEvent>([&](MouseMovedEvent&) {
            calls++;
            return true;
        });

        CHECK_FALSE(dispatched);
        CHECK_EQ(calls, 0);
    }

    TEST_CASE("Handled events are not dispatched again") {
        KeyPressedEvent event(CZ_KEY(B), 0);
        event.SetHandled(true);

        EventDispatcher dispatcher(event);
        const bool dispatched =
            dispatcher.Dispatch<KeyPressedEvent>([](KeyPressedEvent&) { return true; });

        CHECK_FALSE(dispatched);
        CHECK(event.isHandled());
    }

    TEST_CASE("Category flags describe the event family") {
        KeyPressedEvent key(CZ_KEY(C), 0);
        MouseMovedEvent mouse(1.0f, 2.0f);

        CHECK(key.isInCategory(EventCategory_Keyboard));
        CHECK(key.isInCategory(EventCategory_Input));
        CHECK_FALSE(key.isInCategory(EventCategory_Mouse));

        CHECK(mouse.isInCategory(EventCategory_Mouse));
        CHECK_FALSE(mouse.isInCategory(EventCategory_Keyboard));
    }

    TEST_CASE("ToString is not empty") {
        KeyPressedEvent event(CZ_KEY(D), 3);

        CHECK_FALSE(event.ToString().empty());
    }
}

TEST_SUITE("EventBus") {

    TEST_CASE("Listeners receive events of their type only") {
        auto& bus = EventBus::Get();

        int keyHits   = 0;
        int mouseHits = 0;
        bus.AddListener(
            EventType::KeyPressed,
            [&](Event&) {
                keyHits++;
                return false;
            },
            true);
        bus.AddListener(
            EventType::MouseMoved,
            [&](Event&) {
                mouseHits++;
                return false;
            },
            true);

        KeyPressedEvent key(CZ_KEY(E), 0);
        bus.Dispatch(key);

        MouseMovedEvent mouse(3.0f, 4.0f);
        bus.Dispatch(mouse);

        CHECK_EQ(keyHits, 1);
        CHECK_EQ(mouseHits, 1);
    }

    TEST_CASE("A handled event stops the dispatch chain") {
        auto& bus = EventBus::Get();

        int firstHits  = 0;
        int secondHits = 0;
        bus.AddListener(
            EventType::KeyReleased,
            [&](Event&) {
                firstHits++;
                return true;
            },
            true);
        bus.AddListener(
            EventType::KeyReleased,
            [&](Event&) {
                secondHits++;
                return true;
            },
            true);

        KeyReleasedEvent event(CZ_KEY(F));
        bus.Dispatch(event);

        CHECK_EQ(firstHits, 1);
        CHECK_EQ(secondHits, 0);
    }

    TEST_CASE("Destroy-on-dispatch listeners only fire once") {
        auto& bus = EventBus::Get();

        int hits = 0;
        bus.AddListener(
            EventType::KeyTyped,
            [&](Event&) {
                hits++;
                return true;
            },
            true);

        KeyTypedEvent first(CZ_KEY(G));
        bus.Dispatch(first);

        // The listener was removed after the first dispatch.
        KeyTypedEvent second(CZ_KEY(H));
        bus.Dispatch(second);

        CHECK_EQ(hits, 1);
    }
}

#pragma once

#include "Event.h"
#include "KeyCodes.h"

class FKeyEvent : public IEvent {
public:
    EKeyCode GetKeyCode() const { return m_KeyCode; }

    EVENT_CLASS_CATEGORY(EventCategory_Keyboard | EventCategory_Input)
protected:
    explicit FKeyEvent(const EKeyCode keycode) : m_KeyCode(keycode) {}

    EKeyCode m_KeyCode;
};

class FKeyPressedEvent final : public FKeyEvent {
public:
    FKeyPressedEvent(EKeyCode keycode, int repeatCount)
        : FKeyEvent(keycode), m_RepeatCount(repeatCount) {}

    int GetRepeatCount() const { return m_RepeatCount; }

    std::string ToString() const override {
        std::stringstream ss;
        ss << "KeyPressedEvent: " << m_KeyCode << " (" << m_RepeatCount << " repeats)";
        return ss.str();
    }

    EVENT_CLASS_TYPE(KeyPressed)
private:
    int m_RepeatCount;
};

class FKeyReleasedEvent final : public FKeyEvent {
public:
    explicit FKeyReleasedEvent(const EKeyCode keycode) : FKeyEvent(keycode) {}

    std::string ToString() const override {
        std::stringstream ss;
        ss << "KeyReleasedEvent: " << m_KeyCode;
        return ss.str();
    }

    EVENT_CLASS_TYPE(KeyReleased)
};

class FKeyTypedEvent final : public FKeyEvent {
public:
    explicit FKeyTypedEvent(EKeyCode keycode) : FKeyEvent(keycode) {}

    std::string ToString() const override {
        std::stringstream ss;
        ss << "KeyTypedEvent: " << m_KeyCode;
        return ss.str();
    }

    EVENT_CLASS_TYPE(KeyTyped)
};
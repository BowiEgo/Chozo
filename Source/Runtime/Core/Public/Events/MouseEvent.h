#pragma once

#include "Event.h"
#include "KeyCodes.h"

class FMouseMovedEvent final : public IEvent {
public:
    FMouseMovedEvent(float x, float y) : m_MouseX(x), m_MouseY(y) {}

    float GetX() const { return m_MouseX; }
    float GetY() const { return m_MouseY; }

    std::string ToString() const override {
        std::stringstream ss;
        ss << "MouseMovedEvent: " << m_MouseX << ", " << m_MouseY;
        return ss.str();
    }

    EVENT_CLASS_TYPE(MouseMoved)
    EVENT_CLASS_CATEGORY(EventCategory_Mouse | EventCategory_Input)
private:
    float m_MouseX, m_MouseY;
};

class FMouseScrolledEvent final : public IEvent {
public:
    FMouseScrolledEvent(const float xOffset, const float yOffset)
        : m_XOffset(xOffset), m_YOffset(yOffset) {}

    float GetXOffset() const { return m_XOffset; }
    float GetYOffset() const { return m_YOffset; }

    std::string ToString() const override {
        std::stringstream ss;
        ss << "MouseScrolledEvent: " << m_XOffset << ", " << m_YOffset;
        return ss.str();
    }

    EVENT_CLASS_TYPE(MouseScrolled)
    EVENT_CLASS_CATEGORY(EventCategory_Mouse | EventCategory_Input)
private:
    float m_XOffset, m_YOffset;
};

class FMouseButtonEvent : public IEvent {
public:
    EMouseButton GetMouseButton() const { return m_Button; }

    EVENT_CLASS_CATEGORY(EventCategory_Mouse | EventCategory_Input)
protected:
    explicit FMouseButtonEvent(const EMouseButton button) : m_Button(button) {}

    EMouseButton m_Button;
};

class FMouseButtonPressedEvent final : public FMouseButtonEvent {
public:
    explicit FMouseButtonPressedEvent(const EMouseButton button) : FMouseButtonEvent(button) {}

    std::string ToString() const override {
        std::stringstream ss;
        ss << "MouseButtonPressedEvent: " << m_Button;
        return ss.str();
    }

    EVENT_CLASS_TYPE(MouseButtonPressed)
};

class FMouseButtonReleasedEvent final : public FMouseButtonEvent {
public:
    explicit FMouseButtonReleasedEvent(EMouseButton button) : FMouseButtonEvent(button) {}

    std::string ToString() const override {
        std::stringstream ss;
        ss << "MouseButtonReleasedEvent: " << m_Button;
        return ss.str();
    }

    EVENT_CLASS_TYPE(MouseButtonReleased)
};
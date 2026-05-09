#pragma once

#include <Core/Event/Event.h>

#include <sstream>

class WindowContentScaledEvent : public Event {
public:
    WindowContentScaledEvent(float xscale, float yscale) : m_XScale(xscale), m_YScale(yscale) {}

    float GetXScale() const { return m_XScale; }
    float GetYScale() const { return m_YScale; }

    std::string ToString() const override {
        std::stringstream ss;
        ss << "WindowContentScaledEvent: " << m_XScale << ", " << m_YScale;
        return ss.str();
    }

    EVENT_CLASS_TYPE(WindowContentScale)
    EVENT_CLASS_CATEGORY(EventCategory_Application)
private:
    float m_XScale, m_YScale;
};

class WindowResizedEvent : public Event {
public:
    WindowResizedEvent(unsigned int width, unsigned int height)
        : m_Width(width), m_Height(height) {}

    unsigned int GetWidth() const { return m_Width; }
    unsigned int GetHeight() const { return m_Height; }

    std::string ToString() const override {
        std::stringstream ss;
        ss << "WindowResizedEvent: " << m_Width << ", " << m_Height;
        return ss.str();
    }

    EVENT_CLASS_TYPE(WindowResize)
    EVENT_CLASS_CATEGORY(EventCategory_Application)
private:
    unsigned int m_Width, m_Height;
};

class WindowCloseEvent final : public Event {
public:
    WindowCloseEvent() = default;

    EVENT_CLASS_TYPE(WindowClose)
    EVENT_CLASS_CATEGORY(EventCategory_Application)
};

class AppTickEvent final : public Event {
public:
    AppTickEvent() = default;

    EVENT_CLASS_TYPE(AppTick)
    EVENT_CLASS_CATEGORY(EventCategory_Application)
};

class AppUpdateEvent final : public Event {
public:
    AppUpdateEvent() = default;

    EVENT_CLASS_TYPE(AppUpdate)
    EVENT_CLASS_CATEGORY(EventCategory_Application)
};

class AppRenderEvent final : public Event {
public:
    AppRenderEvent() = default;

    EVENT_CLASS_TYPE(AppRender)
    EVENT_CLASS_CATEGORY(EventCategory_Application)
};
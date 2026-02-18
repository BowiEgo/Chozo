#pragma once

#include "Event.h"

class FWindowContentScaledEvent : public IEvent {
public:
    FWindowContentScaledEvent(float xscale, float yscale) : m_XScale(xscale), m_YScale(yscale) {}

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

class FWindowResizedEvent : public IEvent {
public:
    FWindowResizedEvent(unsigned int width, unsigned int height)
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

class FWindowCloseEvent final : public IEvent {
public:
    FWindowCloseEvent() = default;

    EVENT_CLASS_TYPE(WindowClose)
    EVENT_CLASS_CATEGORY(EventCategory_Application)
};

class FAppTickEvent final : public IEvent {
public:
    FAppTickEvent() = default;

    EVENT_CLASS_TYPE(AppTick)
    EVENT_CLASS_CATEGORY(EventCategory_Application)
};

class FAppUpdateEvent final : public IEvent {
public:
    FAppUpdateEvent() = default;

    EVENT_CLASS_TYPE(AppUpdate)
    EVENT_CLASS_CATEGORY(EventCategory_Application)
};

class FAppRenderEvent final : public IEvent {
public:
    FAppRenderEvent() = default;

    EVENT_CLASS_TYPE(AppRender)
    EVENT_CLASS_CATEGORY(EventCategory_Application)
};
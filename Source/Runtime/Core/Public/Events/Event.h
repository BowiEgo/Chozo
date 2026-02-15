#pragma once

#include "CoreTypes.h"

enum class EEventType {
    None = 0,
    WindowClose,
    WindowContentScale,
    WindowResize,
    WindowFocus,
    WindowLostFocus,
    WindowMoved,
    AppTick,
    AppUpdate,
    AppRender,
    SceneRender,
    KeyPressed,
    KeyReleased,
    KeyTyped,
    MouseButtonPressed,
    MouseButtonReleased,
    MouseMoved,
    MouseScrolled
};

enum EEventCategory {
    None = 0,
    EventCategory_Application = BIT(0),
    EventCategory_Render = BIT(1),
    EventCategory_Input = BIT(2),
    EventCategory_Keyboard = BIT(3),
    EventCategory_Mouse = BIT(4),
    EventCategory_MouseButton = BIT(5),
};

#define EVENT_CLASS_TYPE(type)                                                                     \
    static EEventType GetStaticType() { return EEventType::type; }                                 \
    virtual EEventType GetEventType() const override { return GetStaticType(); }                   \
    virtual const char* GetName() const override { return #type; }

#define EVENT_CLASS_CATEGORY(category)                                                             \
    virtual int GetCategoryFlags() const override { return category; }

class IEvent {
    friend class FEventDispatcher;

public:
    virtual ~IEvent() = default;

    virtual EEventType GetEventType() const = 0;
    virtual const char* GetName() const = 0;
    virtual int GetCategoryFlags() const = 0;
    virtual std::string ToString() const { return GetName(); }

    bool isInCategory(const EEventCategory category) const { return GetCategoryFlags() & category; }
    bool isHandled() const { return m_Handled; }
    void SetHandled(const bool handled) { m_Handled = handled; }

public:
    bool m_Handled = false;
};

inline std::ostream& operator<<(std::ostream& os, const IEvent& e) { return os << e.ToString(); };

class FEventDispatcher {
    template <typename T> using TEventFn = std::function<bool(T&)>;

public:
    explicit FEventDispatcher(IEvent& event) : m_Event(event) {}

    template <typename T> bool Dispatch(TEventFn<T> func) {
        if (m_Event.GetEventType() == T::GetStaticType() && !m_Event.m_Handled) {
            m_Event.m_Handled = func(*(T*)&m_Event);
            return true;
        }
        return false;
    }

private:
    IEvent& m_Event;
};

using FEventCallback = std::function<bool(IEvent&)>;

class FEventBus {
    using FEventListener = std::pair<bool, FEventCallback>;

public:
    FEventBus() = default;

    void AddListener(const EEventType type, const FEventCallback& callback, bool destroy = false) {
        m_Listeners[type].emplace_back(destroy, callback);
    }

    void Dispatch(IEvent& event) {
        auto eventType = event.GetEventType();
        auto it = m_Listeners.find(eventType);

        if (it != m_Listeners.end()) {
            for (auto itListener = it->second.begin(); itListener != it->second.end();) {
                bool destroy = std::get<0>(*itListener);
                auto callback = std::get<1>(*itListener);

                if (!event.m_Handled) {
                    event.m_Handled = callback(event);
                    if (destroy)
                        itListener = it->second.erase(itListener);
                } else {
                    ++itListener;
                }
            }
        }
    }

private:
    std::unordered_map<EEventType, std::vector<FEventListener>> m_Listeners;
};
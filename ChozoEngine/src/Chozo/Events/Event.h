#pragma once

#include "Chozo/Core/Base.h"

namespace Chozo {
    enum class EventType
    {
        None = 0,
        WindowClose, WindowResize, WindowFocus, WindowLostFocus, WindowMoved,
        AppTick, AppUpdate, AppRender,
        SceneRender,
        KeyPressed, KeyReleased, KeyTyped,
        MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled
    };

    enum EventCategory
    {
        None = 0,
        EventCategory_Application = BIT(0),
        EventCategory_Render      = BIT(1),
        EventCategory_Input       = BIT(2),
        EventCategory_Keyboard    = BIT(3),
        EventCategory_Mouse       = BIT(4),
        EventCategory_MouseButton = BIT(5),
    };

#define EVENT_CLASS_TYPE(type) static EventType GetStaticType() { return EventType::type; }\
                               virtual EventType GetEventType() const override { return GetStaticType(); }\
                               virtual const char* GetName() const override { return #type; }

#define EVENT_CLASS_CATEGORY(category) virtual int GetCategoryFlags() const override { return category; }

    class Event
    {
        friend class EventDispatcher;
    public:
        virtual ~Event() = default;

        [[nodiscard]] virtual EventType GetEventType() const = 0;
        [[nodiscard]] virtual const char* GetName() const = 0;
        [[nodiscard]] virtual int GetCategoryFlags() const = 0;
        [[nodiscard]] virtual std::string ToString() const { return GetName(); }

        [[nodiscard]] bool isInCategory(const EventCategory category) const { return GetCategoryFlags() & category; }
        [[nodiscard]] bool isHandled() const { return m_Handled; }
        void SetHandled(const bool handled) { m_Handled = handled; }
    public:
        bool m_Handled = false;
    };

    inline std::ostream& operator<<(std::ostream& os, const Event& e)
    {
        return os << e.ToString(); 
    };

    class EventDispatcher
    {
        template<typename T>
        using EventFn = std::function<bool(T&)>;
    public:
        explicit EventDispatcher(Event& event)
            : m_Event(event)
        {
        }

        template<typename T>
        bool Dispatch(EventFn<T> func)
        {
            if (m_Event.GetEventType() == T::GetStaticType() && !m_Event.m_Handled)
            {
                m_Event.m_Handled = func(*(T*)&m_Event);
                return true;
            }
            return false;
        }

    private:
        Event& m_Event;
    };

    using EventCallback = std::function<bool(Event&)>;

    class EventBus
    {
        using EventListener = std::pair<bool, EventCallback>;
    public:
        EventBus() = default;

        void AddListener(const EventType type, const EventCallback& callback, bool destroy = false)
        {
            m_Listeners[type].emplace_back(destroy, callback);
        }

        void Dispatch(Event& event)
        {
            auto eventType = event.GetEventType();
            auto it = m_Listeners.find(eventType);

            if (it != m_Listeners.end())
            {
                for (auto itListener = it->second.begin(); itListener != it->second.end(); )
                {
                    bool destroy = std::get<0>(*itListener);
                    auto callback = std::get<1>(*itListener);

                    if (!event.m_Handled)
                    {
                        event.m_Handled = callback(event);
                        if (destroy)
                            itListener = it->second.erase(itListener);
                    }
                    else
                    {
                        ++itListener;
                    }
                }
            }
        }

    private:
        std::unordered_map<EventType, std::vector<EventListener>> m_Listeners;
    };
}
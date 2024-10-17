#pragma once

#include "Event.h"

namespace Chozo {

    class CHOZO_API SceneRenderEvent : public Event
    {
    public:
        SceneRenderEvent() {}

        EVENT_CLASS_TYPE(SceneRender)
        EVENT_CLASS_CATEGORY(EventCategory_Render)
    };
}
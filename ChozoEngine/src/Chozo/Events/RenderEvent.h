#pragma once

#include "Event.h"

namespace Chozo {

    class SceneRenderEvent final : public Event
    {
    public:
        SceneRenderEvent() = default;

        EVENT_CLASS_TYPE(SceneRender)
        EVENT_CLASS_CATEGORY(EventCategory_Render)
    };
}
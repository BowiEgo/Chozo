#pragma once

#include "czpch.h"

namespace Chozo {

    class RenderCommandBuffer
    {
    public:
        virtual ~RenderCommandBuffer() {};

        virtual void Begin() = 0;
        virtual void End() = 0;

        virtual void Submit() = 0;

        virtual void AddCommand(std::function<void()>&& func) = 0;

        static Ref<RenderCommandBuffer> Create();
    };
}

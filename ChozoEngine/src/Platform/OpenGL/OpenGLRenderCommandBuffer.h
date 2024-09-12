#pragma once

#include "Chozo/Renderer/RenderCommandBuffer.h"

#include "OpenGLUtils.h"
#include <glad/glad.h>

namespace Chozo {

    class OpenGLRenderCommandBuffer : public RenderCommandBuffer
    {
    public:
        OpenGLRenderCommandBuffer();
        virtual ~OpenGLRenderCommandBuffer();

        virtual void Begin() override;
        virtual void End() override;

        virtual void Submit() override;

        virtual void AddCommand(std::function<void()>&& func) override;
    private:
        std::vector<std::function<void()>> m_Commands;
    };
}

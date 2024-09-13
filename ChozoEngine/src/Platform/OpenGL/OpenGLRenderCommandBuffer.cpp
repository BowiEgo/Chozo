#include "OpenGLRenderCommandBuffer.h"

#include "Chozo/Renderer/Renderer.h"

namespace Chozo {

    OpenGLRenderCommandBuffer::OpenGLRenderCommandBuffer()
    {
    }

    OpenGLRenderCommandBuffer::~OpenGLRenderCommandBuffer()
    {
    }

    void OpenGLRenderCommandBuffer::Begin()
    {
        m_Commands.clear();
    }

    void OpenGLRenderCommandBuffer::End()
    {
    }

    void OpenGLRenderCommandBuffer::Submit()
    {
        for (auto& cmd : m_Commands)
            Renderer::Submit(std::move(cmd));
    }

    void OpenGLRenderCommandBuffer::AddCommand(std::function<void()> &&func)
    {
        m_Commands.push_back([func = std::forward<std::function<void()>>(func)]() mutable { func(); });
    }
}

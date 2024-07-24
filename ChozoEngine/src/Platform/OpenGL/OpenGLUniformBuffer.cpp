#include "OpenGLUniformBuffer.h"
#include "Chozo/Renderer/Renderer2D.h"

#include <glad/glad.h>

namespace Chozo {
    unsigned int OpenGLUniformBuffer::s_UniformBindingPoint = -1;

    OpenGLUniformBuffer::OpenGLUniformBuffer(uint32_t size)
    {
        glGenBuffers(1, &m_RendererID);

        glBindBuffer(GL_UNIFORM_BUFFER, m_RendererID);
        glBufferData(GL_UNIFORM_BUFFER, size, NULL, GL_STATIC_DRAW);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        m_BindingPoint = ++s_UniformBindingPoint;
        glBindBufferRange(GL_UNIFORM_BUFFER, m_BindingPoint, m_RendererID, 0, size);
    }

    void OpenGLUniformBuffer::SetData(const void* data, uint32_t size, uint32_t offset)
    {
        glBindBuffer(GL_UNIFORM_BUFFER, m_RendererID);
        glBufferSubData(GL_UNIFORM_BUFFER, offset, size, data);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }
}

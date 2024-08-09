#include "OpenGLIndexBuffer.h"

#include "OpenGLUtils.h"

#include <glad/glad.h>

namespace Chozo {
    
    OpenGLIndexBuffer::OpenGLIndexBuffer(void* indices, uint32_t count)
        : m_Count(count)
    {
        glGenBuffers(1, &m_RendererID); GCE;
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID); GCE;
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(uint32_t), indices, GL_STATIC_DRAW); GCE;
    }

    OpenGLIndexBuffer::~OpenGLIndexBuffer()
    {
        glDeleteBuffers(1, &m_RendererID); GCE;
    }

    void OpenGLIndexBuffer::SetData(uint32_t offset, uint32_t count, void* indices)
    {
        Bind();
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, offset * sizeof(uint32_t), count * sizeof(uint32_t), indices); GCE;
        m_Count = count;
        m_End = std::max((uint32_t)(count * sizeof(uint32_t)), m_End);
    }

    void OpenGLIndexBuffer::ClearData()
    {
        Bind();
        uint32_t indices[m_End / sizeof(uint32_t)];
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, m_End, indices); GCE;
    }

    void OpenGLIndexBuffer::Resize(uint32_t count)
    {
        Bind();
        uint32_t indices[count];
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(uint32_t), indices, GL_STATIC_DRAW); GCE;
    }

    void OpenGLIndexBuffer::Bind() const
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID); GCE;
    }

    void OpenGLIndexBuffer::Unbind() const
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); GCE;
    }
}
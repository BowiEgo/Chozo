#include "OpenGLVertexBuffer.h"

#include <glad/glad.h>

namespace Chozo {

    OpenGLVertexBuffer::OpenGLVertexBuffer(uint32_t size)
    {
        glGenBuffers(1, &m_RendererID);
        glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
        glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
    }

    OpenGLVertexBuffer::OpenGLVertexBuffer(void* vertices, uint32_t size)
    {
        glGenBuffers(1, &m_RendererID);
        glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
        glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);
    }

    OpenGLVertexBuffer::~OpenGLVertexBuffer()
    {
        glDeleteBuffers(1, &m_RendererID);
    }

    void OpenGLVertexBuffer::SetData(void* vertices, uint32_t size)
    {
        Bind();
        glBufferSubData(GL_ARRAY_BUFFER, 0, size, vertices);
        m_Offset = std::max(size, m_Offset);
    }

    void OpenGLVertexBuffer::ClearData()
    {
        Bind();
        std::vector<float> zeroData(m_Offset / sizeof(float), 0.0f);
        glBufferSubData(GL_ARRAY_BUFFER, 0, m_Offset, zeroData.data());
    }

    void OpenGLVertexBuffer::Bind() const
    {
        glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
    }

    void OpenGLVertexBuffer::Unbind() const
    {
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
}
#include "OpenGLIndexBuffer.h"

#include <glad/glad.h>

namespace Chozo {
    
    OpenGLIndexBuffer::OpenGLIndexBuffer(void* indices, uint32_t count)
        : m_Count(count)
    {
        glGenBuffers(1, &m_RendererID);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(uint32_t), indices, GL_STATIC_DRAW);
    }

    OpenGLIndexBuffer::~OpenGLIndexBuffer()
    {
        glDeleteBuffers(1, &m_RendererID);
    }

    void OpenGLIndexBuffer::SetData(void* indices, uint32_t size)
    {
        Bind();
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, size, indices);
        m_Offset = std::max(size, m_Offset);
    }

    void OpenGLIndexBuffer::ClearData()
    {
        Bind();
        std::vector<unsigned int> zeroData(m_Offset / sizeof(unsigned int), 0.0f);
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, m_Offset, zeroData.data());
    }

    void OpenGLIndexBuffer::Bind() const
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);
    }

    void OpenGLIndexBuffer::Unbind() const
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }
}
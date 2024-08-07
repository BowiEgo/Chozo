#include "OpenGLVertexBuffer.h"

#include "OpenGLUtils.h"
#include <glad/glad.h>

namespace Chozo {

    OpenGLVertexBuffer::OpenGLVertexBuffer(uint32_t size)
    {
        glGenBuffers(1, &m_RendererID); GCE;
        glBindBuffer(GL_ARRAY_BUFFER, m_RendererID); GCE;
        glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_DYNAMIC_DRAW); GCE;
    }

    OpenGLVertexBuffer::OpenGLVertexBuffer(void* vertices, uint32_t size)
    {
        glGenBuffers(1, &m_RendererID); GCE;
        glBindBuffer(GL_ARRAY_BUFFER, m_RendererID); GCE;
        glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW); GCE;
    }

    OpenGLVertexBuffer::~OpenGLVertexBuffer()
    {
        glDeleteBuffers(1, &m_RendererID); GCE;
    }

    void OpenGLVertexBuffer::GetData(uint32_t offset, uint32_t size)
    {
        Bind();
        void* outData;
        // glGetBufferSubData(GL_ARRAY_BUFFER, offset, size, outData);
        Unbind();
    }

    void OpenGLVertexBuffer::SetData(uint32_t offset, uint32_t size, void* vertices)
    {
        Bind();
        glBufferSubData(GL_ARRAY_BUFFER, offset, size, vertices); GCE;
        m_Offset = std::max(size, m_Offset);
    }

    void OpenGLVertexBuffer::ClearData()
    {
        Bind();
        std::vector<float> zeroData(m_Offset / sizeof(float), 0.0f);
        glBufferSubData(GL_ARRAY_BUFFER, 0, m_Offset, zeroData.data()); GCE;
    }

    void OpenGLVertexBuffer::Resize(uint32_t size)
    {
        Bind();
        std::vector<float> zeroData(size, 0.0f);
        glBufferData(GL_ARRAY_BUFFER, size, zeroData.data(), GL_STATIC_DRAW); GCE;
    }

    void OpenGLVertexBuffer::Bind() const
    {
        glBindBuffer(GL_ARRAY_BUFFER, m_RendererID); GCE;
    }

    void OpenGLVertexBuffer::Unbind() const
    {
        glBindBuffer(GL_ARRAY_BUFFER, 0); GCE;
    }
}
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
        if (size % sizeof(float) != 0) {
            throw std::invalid_argument("Data size must be aligned to float size");
        }

        Bind();
        glBufferSubData(GL_ARRAY_BUFFER, offset, size, vertices); GCE;
        m_End = std::max(offset + size, m_End);
    }

    void OpenGLVertexBuffer::ClearData()
    {
        if (m_End == 0) return;

        Bind();

        GLint boundBuffer = 0;
        glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &boundBuffer);
        if (boundBuffer != static_cast<GLint>(m_RendererID)) {
            throw std::runtime_error("Buffer bind failed");
        }

        const size_t floatCount = m_End / sizeof(float);
        if (floatCount * sizeof(float) != m_End) {
            throw std::invalid_argument("m_End not aligned to float size");
        }

        const std::vector<float> zeroData(floatCount, 0.0f);
        glBufferSubData(GL_ARRAY_BUFFER, 0, m_End, zeroData.data()); GCE;
    }

    void OpenGLVertexBuffer::Resize(uint32_t size)
    {
        size = (size + sizeof(float) - 1) & ~(sizeof(float) - 1);
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
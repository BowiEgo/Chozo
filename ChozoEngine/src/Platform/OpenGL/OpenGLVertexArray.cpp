#include "OpenGLVertexArray.h"

#include "OpenGLUtils.h"
#include <glad/glad.h>

namespace Chozo {

    static GLenum ShaderDataTypeToOpenGLBaseType(ShaderDataType type)
    {
        switch (type)
        {
            case Chozo::ShaderDataType::None:    return 0;
            case Chozo::ShaderDataType::Float:   return GL_FLOAT;
            case Chozo::ShaderDataType::Float2:  return GL_FLOAT;
            case Chozo::ShaderDataType::Float3:  return GL_FLOAT;
            case Chozo::ShaderDataType::Float4:  return GL_FLOAT;
            case Chozo::ShaderDataType::Mat3:    return GL_FLOAT;
            case Chozo::ShaderDataType::Mat4:    return GL_FLOAT;
            case Chozo::ShaderDataType::Int:     return GL_INT;
            case Chozo::ShaderDataType::Int2:    return GL_INT;
            case Chozo::ShaderDataType::Int3:    return GL_INT;
            case Chozo::ShaderDataType::Int4:    return GL_INT;
            case Chozo::ShaderDataType::Bool:    return GL_BOOL;
        }

        CZ_CORE_ASSERT(false, "Unknown ShaderDataType!");
        return 0;
    }

    OpenGLVertexArray::OpenGLVertexArray()
    {
        glGenVertexArrays(1, &m_RendererID); GCE;
    }

    OpenGLVertexArray::~OpenGLVertexArray()
    {
        glDeleteVertexArrays(1, &m_RendererID); GCE;
    }

    void OpenGLVertexArray::Bind() const
    {
        glBindVertexArray(m_RendererID); GCE;
    }

    void OpenGLVertexArray::Unbind() const
    {
        glBindVertexArray(0); GCE;
    }

    void OpenGLVertexArray::AddVertexBuffer(const Ref<VertexBuffer>& vertexBuffer)
    {
        CZ_CORE_ASSERT(vertexBuffer->GetLayout().GetElements().size(), "Vertex Buffer has no layout!");

        glBindVertexArray(m_RendererID); GCE;
        vertexBuffer->Bind();

        uint32_t index = 0;
        for (const auto& element : vertexBuffer->GetLayout())
        {
            glEnableVertexAttribArray(index); GCE;
            glVertexAttribPointer(index,
                element.GetComponentCount(),
                ShaderDataTypeToOpenGLBaseType(element.Type),
                element.Normalized ? GL_TRUE : GL_FALSE,
                vertexBuffer->GetLayout().GetStride(),
                reinterpret_cast<const void*>(static_cast<uintptr_t>(element.Offset))
            ); GCE;
            index++;
        }

        m_VertexBuffers.push_back(vertexBuffer);
    }

    void OpenGLVertexArray::SetIndexBuffer(const Ref<IndexBuffer>& indexBuffer)
    {
        glBindVertexArray(m_RendererID); GCE;
        indexBuffer->Bind();

        m_IndexBuffer = indexBuffer;
    }
}
#include "OpenGLVertexArray.h"

#include <glad/glad.h>

namespace Parallax {

    static GLenum ShaderDataTypeToOpenGLBaseType(ShaderDataType type)
    {
        switch (type)
        {
            case Parallax::ShaderDataType::None:    return 0;
            case Parallax::ShaderDataType::Float:   return GL_FLOAT;
            case Parallax::ShaderDataType::Float2:  return GL_FLOAT;
            case Parallax::ShaderDataType::Float3:  return GL_FLOAT;
            case Parallax::ShaderDataType::Float4:  return GL_FLOAT;
            case Parallax::ShaderDataType::Mat3:    return GL_FLOAT;
            case Parallax::ShaderDataType::Mat4:    return GL_FLOAT;
            case Parallax::ShaderDataType::Int:     return GL_INT;
            case Parallax::ShaderDataType::Int2:    return GL_INT;
            case Parallax::ShaderDataType::Int3:    return GL_INT;
            case Parallax::ShaderDataType::Int4:    return GL_INT;
            case Parallax::ShaderDataType::Bool:    return GL_BOOL;
        }

        PRX_CORE_ASSERT(false, "Unknown ShaderDataType!");
        return 0;
    }

    OpenGLVertexArray::OpenGLVertexArray()
    {
        glGenVertexArrays(1, &m_RendererID);
    }

    OpenGLVertexArray::~OpenGLVertexArray()
    {
        glDeleteVertexArrays(1, &m_RendererID);
    }

    void OpenGLVertexArray::Bind() const
    {
        glBindVertexArray(m_RendererID);
    }

    void OpenGLVertexArray::Unbind() const
    {
        glBindVertexArray(0);
    }

    void OpenGLVertexArray::AddVertexBuffer(const Ref<VertexBuffer>& vertexBuffer)
    {
        PRX_CORE_ASSERT(vertexBuffer->GetLayout().GetElements().size(), "Vertex Buffer has no layout!");

        glBindVertexArray(m_RendererID);
        vertexBuffer->Bind();

        uint32_t index = 0;
        for (const auto& element : vertexBuffer->GetLayout())
        {
            glEnableVertexAttribArray(index);
            glVertexAttribPointer(index,
                element.GetComponentCount(),
                ShaderDataTypeToOpenGLBaseType(element.Type),
                element.Normalized ? GL_TRUE : GL_FALSE,
                vertexBuffer->GetLayout().GetStride(),
                reinterpret_cast<const void*>(static_cast<uintptr_t>(element.Offset))
            );
            index++;
        }

        m_VertexBuffers.push_back(vertexBuffer);
    }

    void OpenGLVertexArray::SetIndexBuffer(const Ref<IndexBuffer>& indexBuffer)
    {
        glBindVertexArray(m_RendererID);
        indexBuffer->Bind();

        m_IndexBuffer = indexBuffer;
    }
}
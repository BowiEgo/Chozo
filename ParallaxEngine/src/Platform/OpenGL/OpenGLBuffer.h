#pragma once

#include "Parallax/Renderer/Buffer.h"

namespace Parallax {

    class OpenGLVertexBuffer : public VertexBuffer
    {
    private:
        u_int32_t m_RendererID;
    public:
        OpenGLVertexBuffer(float* vertices, uint32_t size);
        virtual ~OpenGLVertexBuffer();

        virtual void Bind() const;
        virtual void Unbind() const;
    };

    class OpenGLIndexBuffer : public IndexBuffer
    {
    private:
        u_int32_t m_RendererID;
        u_int32_t m_Count;;
    public:
        OpenGLIndexBuffer(uint32_t* indices, uint32_t count);
        virtual ~OpenGLIndexBuffer();

        virtual void Bind() const;
        virtual void Unbind() const;

        virtual uint32_t GetCount() const { return m_Count; }
    };
}
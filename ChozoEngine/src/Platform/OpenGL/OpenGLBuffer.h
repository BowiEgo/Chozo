#pragma once

#include "Chozo/Renderer/Buffer.h"

namespace Chozo {

    class OpenGLVertexBuffer : public VertexBuffer
    {
    private:
        u_int32_t m_RendererID;
        BufferLayout m_Layout;
    public:
        OpenGLVertexBuffer(float* vertices, uint32_t size);
        virtual ~OpenGLVertexBuffer();

        virtual void Bind() const override;
        virtual void Unbind() const override;

        virtual const BufferLayout& GetLayout() const override { return m_Layout; };
        virtual void SetLayout(const BufferLayout& layout) override { m_Layout = layout; };
    };

    class OpenGLIndexBuffer : public IndexBuffer
    {
    private:
        u_int32_t m_RendererID;
        u_int32_t m_Count;;
    public:
        OpenGLIndexBuffer(uint32_t* indices, uint32_t count);
        virtual ~OpenGLIndexBuffer();

        virtual void Bind() const override;
        virtual void Unbind() const override;

        virtual uint32_t GetCount() const override { return m_Count; }
    };
}
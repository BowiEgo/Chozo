#pragma once

#include "Chozo/Renderer/Buffer.h"
#include "Chozo/Renderer/RendererTypes.h"

namespace Chozo {

    class OpenGLVertexBuffer : public VertexBuffer
    {
    private:
        uint32_t m_RendererID;
        BufferLayout m_Layout;
    public:
        OpenGLVertexBuffer();
        OpenGLVertexBuffer(uint32_t size);
        OpenGLVertexBuffer(float* vertices, uint32_t size);
        virtual ~OpenGLVertexBuffer();

        virtual void SetData(float* vertices, uint32_t size) override;

        virtual void Bind() const override;
        virtual void Unbind() const override;

        virtual const RendererID GetRendererID() const override { return m_RendererID; }
        virtual const BufferLayout& GetLayout() const override { return m_Layout; }
        virtual void SetLayout(const BufferLayout& layout) override { m_Layout = layout; }
    };

    class OpenGLIndexBuffer : public IndexBuffer
    {
    private:
        uint32_t m_RendererID;
        uint32_t m_Count;;
    public:
        OpenGLIndexBuffer(uint32_t* indices, uint32_t count);
        virtual ~OpenGLIndexBuffer();

        virtual void Bind() const override;
        virtual void Unbind() const override;

        virtual uint32_t GetCount() const override { return m_Count; }
    };
}
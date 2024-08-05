#pragma once

#include "Chozo/Renderer/VertexBuffer.h"
#include "Chozo/Renderer/RendererTypes.h"

namespace Chozo {

    class OpenGLVertexBuffer : public VertexBuffer
    {
    public:
        OpenGLVertexBuffer(uint32_t size);
        OpenGLVertexBuffer(void* vertices, uint32_t size);
        virtual ~OpenGLVertexBuffer();

        virtual void SetData(void* vertices, uint32_t size) override;
        virtual void ClearData() override;

        virtual void Bind() const override;
        virtual void Unbind() const override;

        virtual const RendererID GetRendererID() const override { return m_RendererID; }
        virtual const VertexBufferLayout& GetLayout() const override { return m_Layout; }
        virtual void SetLayout(const VertexBufferLayout& layout) override { m_Layout = layout; }
    private:
        uint32_t m_RendererID;
        VertexBufferLayout m_Layout;
        uint32_t m_Offset;
    };

}
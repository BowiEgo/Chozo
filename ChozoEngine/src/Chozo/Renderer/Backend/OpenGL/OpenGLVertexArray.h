#pragma once

#include "Chozo/Renderer/VertexBuffer.h"
#include "Chozo/Renderer/IndexBuffer.h"
#include "Chozo/Renderer/VertexArray.h"

namespace Chozo {

    class OpenGLVertexArray : public VertexArray
    {
    private:
        uint32_t m_RendererID;
        std::vector<Ref<VertexBuffer>> m_VertexBuffers;
        Ref<IndexBuffer> m_IndexBuffer;
    public:
        OpenGLVertexArray();
        virtual ~OpenGLVertexArray();

        virtual void Bind() const override;
        virtual void Unbind() const override;

        virtual void AddVertexBuffer(const Ref<VertexBuffer>& vertexBuffer) override;
    	virtual void SetIndexBuffer(const Ref<IndexBuffer>& indexBuffer) override;
        
        virtual const RendererID GetRendererID() const override { return m_RendererID; }
        virtual const std::vector<Ref<VertexBuffer>>& GetVertexBuffers() const override { return m_VertexBuffers; }
        virtual const Ref<IndexBuffer>& GetIndexBuffer() const override { return m_IndexBuffer; }
    };
}
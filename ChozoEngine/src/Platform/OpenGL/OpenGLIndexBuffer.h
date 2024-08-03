#pragma once

#include "Chozo/Renderer/IndexBuffer.h"

namespace Chozo {

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
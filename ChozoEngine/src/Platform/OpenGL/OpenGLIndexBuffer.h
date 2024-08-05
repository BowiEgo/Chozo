#pragma once

#include "Chozo/Renderer/IndexBuffer.h"

namespace Chozo {

    class OpenGLIndexBuffer : public IndexBuffer
    {
    public:
        OpenGLIndexBuffer(void* indices, uint32_t count);
        virtual ~OpenGLIndexBuffer();

        virtual void SetData(void* indices, uint32_t size) override;
        virtual void ClearData() override;

        virtual void Bind() const override;
        virtual void Unbind() const override;

        virtual uint32_t GetCount() const override { return m_Count; }
    private:
        uint32_t m_RendererID;
        uint32_t m_Count;
        uint32_t m_Offset;
    };
}
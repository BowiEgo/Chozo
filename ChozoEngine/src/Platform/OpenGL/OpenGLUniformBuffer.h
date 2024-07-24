#pragma once

#include "Chozo/Renderer/UniformBuffer.h"

namespace Chozo {

    class OpenGLUniformBuffer : public UniformBuffer
    {
    public:
        OpenGLUniformBuffer(uint32_t size);
        virtual ~OpenGLUniformBuffer() override {}

        virtual void SetData(const void* data, uint32_t size, uint32_t offset = 0) override;
    private:
        static unsigned int s_UniformBindingPoint;
        unsigned int m_BindingPoint;
        uint32_t m_RendererID;
    };
}
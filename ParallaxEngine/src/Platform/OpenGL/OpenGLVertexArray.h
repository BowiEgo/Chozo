#pragma once

#include "Parallax/Renderer/VertexArray.h"

namespace Parallax {

    class OpenGLVertexArray : public VertexArray
    {
    private:
        u_int32_t m_RendererID;
    public:
        OpenGLVertexArray();
        virtual ~OpenGLVertexArray();

        virtual void AddBuffer(uint32_t size) const;

        virtual void Bind() const;
        virtual void Unbind() const;
    };
}
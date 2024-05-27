#pragma once

#include "prxpch.h"

namespace Parallax {

    class VertexArray
    {
    public:
        virtual ~VertexArray() {}

    	virtual void AddBuffer(uint32_t size) const = 0;

        virtual void Bind() const = 0;
        virtual void Unbind() const = 0;

        static VertexArray* Create();
    };
}
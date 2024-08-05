#pragma once

#include "czpch.h"

#include "RendererTypes.h"
namespace Chozo {

    class IndexBuffer
    {
    public:
        virtual ~IndexBuffer() {}

        virtual void SetData(void* indices, uint32_t size) = 0;
        virtual void ClearData() = 0;

        virtual void Bind() const = 0;
        virtual void Unbind() const = 0;

        virtual uint32_t GetCount() const = 0;

        static Ref<IndexBuffer> Create(void* indices, uint32_t size);
    };
}

#pragma once

#include <Core/Header/Handle.hpp>
#include <Core/Memory/Buffer.hpp>
#include <Core/Memory/Memory.hpp>
#include <Runtime/RHI/RHIResource.hpp>
#include <Runtime/RHI/RHITypes.hpp>

namespace CZ {

struct GraphicsBufferSpecification {
    size_t Size           = 0;
    BufferUsage Usage     = BufferUsage::None;
    MemoryType MemoryType = MemoryType::Unknown;
    std::string Name; // For debugging

    // Alignment requirements (0 = default)
    size_t MinAlignment = 0;
};

class GraphicsBufferObj : public RHIResource {
public:
    GraphicsBufferObj(const GraphicsBufferSpecification& spec) : m_Spec(spec) {}
    virtual ~GraphicsBufferObj() = default;

    // Disable copy
    GraphicsBufferObj(const GraphicsBufferObj&)            = delete;
    GraphicsBufferObj& operator=(const GraphicsBufferObj&) = delete;

    ResourceType GetResourceType() const override { return ResourceType::GraphicsBuffer; }

    // Core functionality
    virtual void* Map(size_t offset = 0, size_t size = 0)       = 0;
    virtual void Unmap()                                        = 0;
    // Upload data (convenience)
    virtual void SetData(const Buffer* data, size_t offset = 0) = 0;

    // Getters
    virtual size_t GetSize() const { return m_Spec.Size; }
    virtual BufferUsage GetUsage() const { return m_Spec.Usage; }
    virtual MemoryType GetMemoryType() const { return m_Spec.MemoryType; }
    virtual const std::string& GetName() const { return m_Spec.Name; }

protected:
    GraphicsBufferSpecification m_Spec;
};

struct GraphicsBuffer : Handle<class GraphicsBufferObj> {
    template <typename T> T* As() { return static_cast<T*>(InternalHandleReader::Unwrap(*this)); }

    template <typename T> const T* As() const {
        return static_cast<const T*>(InternalHandleReader::Unwrap(*this));
    }

    GraphicsBufferObj* Raw() { return InternalHandleReader::Unwrap(*this); }
};

} // namespace CZ
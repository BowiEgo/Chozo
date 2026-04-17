#pragma once

#include "Buffer.h"
#include "CoreMinimal.h"
#include "RHIExport.h"
#include "RHIResource.h"
#include "RHITypes.h"

DECLARE_LOG_CATEGORY_EXTERN(LogRHIBuffer, Info);

struct FBufferSpecification {
    size_t Size            = 0;
    EBufferUsage Usage     = EBufferUsage::None;
    EMemoryType MemoryType = EMemoryType::Unknown;
    std::string Name; // For debugging

    // Alignment requirements (0 = default)
    size_t MinAlignment = 0;
};

class RHI_API IRHIBuffer : public IRHIResource {
public:
    IRHIBuffer(const WeakRef<IRHIDevice> device, const FBufferSpecification& spec)
        : IRHIResource(device), m_Spec(spec) {}
    virtual ~IRHIBuffer() {}

    // Disable copy
    IRHIBuffer(const IRHIBuffer&)            = delete;
    IRHIBuffer& operator=(const IRHIBuffer&) = delete;

    // Core functionality
    virtual void* Map(size_t offset = 0, size_t size = 0) = 0;
    virtual void Unmap()                                  = 0;

    // Upload data (convenience)
    virtual void SetData(FBuffer& data, size_t offset = 0) = 0;

    // Getters
    virtual size_t GetSize() const { return m_Spec.Size; }
    virtual EBufferUsage GetUsage() const { return m_Spec.Usage; }
    virtual EMemoryType GetMemoryType() const { return m_Spec.MemoryType; }
    virtual const std::string& GetName() const { return m_Spec.Name; }

protected:
    FBufferSpecification m_Spec;
};
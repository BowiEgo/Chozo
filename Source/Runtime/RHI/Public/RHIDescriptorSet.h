#pragma once

#include "RHIExport.h"
#include "RHIResource.h"
#include "Ref.h"

DECLARE_LOG_CATEGORY_EXTERN(LogRHIDescriptorSet, Info);

class RHI_API IRHIDescriptorSet : public IRHIResource {
public:
    IRHIDescriptorSet(const WeakRef<IRHIDevice> device);
    virtual ~IRHIDescriptorSet();

    virtual void* GetRawHandle() const = 0;
};
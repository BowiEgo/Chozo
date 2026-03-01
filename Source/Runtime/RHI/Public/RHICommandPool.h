#pragma once

#include "RHIExport.h"
#include "RHITypes.h"
#include "Ref.h"

DECLARE_LOG_CATEGORY_EXTERN(LogRHICommandPool, Info);

struct FCommandPoolSpecification {
    uint32 QueueIndex;
    ECommandPoolFlags Flags;
};

class IRHICommandList;

class RHI_API IRHICommandPool : public FRefCounted {
public:
    IRHICommandPool(const FCommandPoolSpecification& spec);
    virtual ~IRHICommandPool();

    virtual TRef<IRHICommandList> AllocateCommandBuffer() = 0;

protected:
    FCommandPoolSpecification m_Spec;
};

#pragma once

#include "RHICommandPool.h"
#include "RHIExport.h"
#include "Ref.h"

DECLARE_LOG_CATEGORY_EXTERN(LogRHICommandBuffer, Info);

struct FRHICommandBufferCreateInfo {
    TRef<IRHICommandPool> CommandPool;
};

class RHI_API IRHICommandBuffer : public FRefCounted {
public:
    IRHICommandBuffer(const FRHICommandBufferCreateInfo& info);
    virtual ~IRHICommandBuffer();

protected:
    FRHICommandBufferCreateInfo m_Info;
};

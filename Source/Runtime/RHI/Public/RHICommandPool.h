#pragma once

#include "RHICommandList.h"
#include "RHIDevice.h"
#include "RHIExport.h"
#include "Ref.h"

DECLARE_LOG_CATEGORY_EXTERN(LogRHICommandPool, Info);

struct FRHICommandPoolCreateInfo {
    uint32 QueueIndex;
};

class IRHICommandBuffer;

class RHI_API IRHICommandPool : public FRefCounted {
public:
    IRHICommandPool(const FRHICommandPoolCreateInfo& info);
    virtual ~IRHICommandPool();

    virtual TRef<IRHICommandList> AllocateCommandList(const FRHICommandListCreateInfo& info,
                                                      const TRef<IRHICommandBuffer> cmdBuffer) = 0;

protected:
    TRef<IRHICommandList> m_CommandList;

    FRHICommandPoolCreateInfo m_Info;
};

#pragma once

#include "RHIExport.h"
#include "Ref.h"

DECLARE_LOG_CATEGORY_EXTERN(LogRHICommandList, Info);

struct FRHICommandListCreateInfo {};

class RHI_API IRHICommandList : public FRefCounted {
public:
    IRHICommandList(const FRHICommandListCreateInfo& info);
    virtual ~IRHICommandList();

    virtual void Begin() = 0;
    virtual void End() = 0;

protected:
    FRHICommandListCreateInfo m_Info;
};

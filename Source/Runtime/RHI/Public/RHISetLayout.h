#pragma once

#include "RHIExport.h"
#include "RHIResource.h"

#include "Ref.h"

DECLARE_LOG_CATEGORY_EXTERN(LogRHISetLayout, Info);

class RHI_API IRHISetLayout : public IRHIResource {
public:
    IRHISetLayout(const WeakRef<IRHIDevice> device);
    virtual ~IRHISetLayout();
};
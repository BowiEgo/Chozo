#pragma once

#include "RHIExport.h"

#include "Ref.h"

DECLARE_LOG_CATEGORY_EXTERN(LogRHISampler, Info);

class RHI_API IRHISampler : public IRHIResource {
public:
    IRHISampler();

    virtual ~IRHISampler();
};
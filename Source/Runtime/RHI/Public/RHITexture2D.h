#pragma once

#include "Buffer.h"
#include "RHIExport.h"
#include "RHIResource.h"
#include "RHITexture.h"
#include "RHITypes.h"
#include "Ref.h"

DECLARE_LOG_CATEGORY_EXTERN(LogRHITexture2D, Info);

class RHI_API IRHITexture2D : public IRHITexture {
public:
    using IRHITexture::IRHITexture;
    virtual ~IRHITexture2D();
};
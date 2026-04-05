#pragma once

#include "Buffer.h"
#include "RHIExport.h"
#include "RHITexture.h"
#include "RHITypes.h"
#include "Ref.h"

DECLARE_LOG_CATEGORY_EXTERN(LogRHITextureCubemap, Info);

class RHI_API IRHITextureCubemap : public IRHITexture {
public:
    using IRHITexture::IRHITexture;
    virtual ~IRHITextureCubemap();

    // // Set data for 6 faces (usually in order: +X, -X, +Y, -Y, +Z, -Z)
    // virtual void SetData(const std::array<FBuffer, 6>& faceBuffers) = 0;
    // // or a single buffer version containing all faces data
    // virtual void SetData(const FBuffer& buffer)                     = 0;

    // virtual void* GetDescriptorSet()       = 0;
    // virtual void* GetDescriptorSet() const = 0;
};
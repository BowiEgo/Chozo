#pragma once

#include "Scope.h"

#include "RHIExport.h"
#include "RHIImage.h"

DECLARE_LOG_CATEGORY_EXTERN(LogImagePool, Info);

struct FPooledImage {
    TScope<IRHIImage> Image;
    uint32_t LastUsedFrame = 0;
    bool bInUse            = false;
};

class RHI_API CImagePool {
public:
    CImagePool(const WeakRef<IRHIDevice> device) : m_Device(device) {};
    ~CImagePool();

    CImagePool(const CImagePool&)            = delete;
    CImagePool& operator=(const CImagePool&) = delete;

    IRHIImage* RequestImage(const FImageSpecification& spec, uint32_t frameIndex);

    // Usually called when RenderGraph destructs, to return images back to the pool for potential
    // reuse or eventual cleanup
    void ReturnImage(IRHIImage* image);

    // This should be called every frame to clean up long-unused resources (e.g., destroy if not
    // used for over 8 frames) and update the status of idle resources
    void Tick(uint32_t currentFrame);
    void Clear();

protected:
    WeakRef<IRHIDevice> m_Device;
    // Store idle images that can potentially be reused, along with their last used frame index
    std::vector<FPooledImage> m_AvailableImages;

    // Max idle frames before destruction, can be tuned based on typical frame times and resource
    // usage patterns
    const uint32_t m_MaxIdleFrames = 8;
};
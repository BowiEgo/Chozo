#pragma once

#include "ImagePool.h"
#include "RHICommandPool.h"
#include "RHIExport.h"
#include "RHIPipeline.h"
#include "RHISyncObject.h"
#include "Ref.h"

DECLARE_LOG_CATEGORY_EXTERN(LogRHIDevice, Info);

class IRHIContext;
class IRHIResource;
class IRHIImage;
class IRHIShader;
struct FShaderSpecification;

struct FDeviceSpecification {
    // --- Debugging ---
    bool bEnableValidationLayers = true;
    bool bEnableGPUProfiling     = false;

    // --- Metadata ---
    std::string AppName;
    uint32_t AppVersion;

    // --- Feature Toggles ---
    // [Note] High-level feature requests that RHI will try to fulfill
    bool bPreferIntegratedGPU = false; // Whether to use iGPU for power saving
    bool bRequireRayTracing   = false;
};

struct DeferredCleanup {
    std::function<void()> CleanupFunc;
    uint32_t RetireFrame;
};

class RHI_API IRHIDevice : public FRefCounted {
public:
    IRHIDevice(const IRHIContext* ctx, const FDeviceSpecification& spec);
    virtual ~IRHIDevice();

    /**
     * [Note] Wait for all GPU tasks to complete before destruction
     */
    virtual void WaitIdle() = 0;

    virtual TRef<IRHICommandPool> CreateCommandPool(FCommandPoolSpecification& spec) = 0;
    virtual TRef<IRHIImage> CreateImage(const FImageSpecification& spec)             = 0;

    CImagePool& GetImagePool() { return m_ImagePool; }

    // Enqueue a cleanup function to be called after it's safe to delete the resource (e.g., after
    // GPU is done with it)
    void EnqueueCleanup(std::function<void()>&& func);
    void TickDeferredDeletion(uint32 currentFrame);

protected:
    FDeviceSpecification m_Spec;

    const IRHIContext* m_Context;
    std::vector<DeferredCleanup> m_DeletionQueue;

    CImagePool m_ImagePool;
};

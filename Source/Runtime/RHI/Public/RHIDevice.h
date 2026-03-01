#pragma once

#include "RHICommandPool.h"
#include "RHIExport.h"
#include "RHIPipeline.h"
#include "RHISyncObject.h"
#include "Ref.h"

DECLARE_LOG_CATEGORY_EXTERN(LogRHIDevice, Info);

class IRHIShader;
struct FShaderSpecification;

struct FDeviceSpecification {
    // --- Debugging ---
    bool bEnableValidationLayers = true;
    bool bEnableGPUProfiling = false;

    // --- Metadata ---
    std::string AppName;
    uint32_t AppVersion;

    // --- Feature Toggles ---
    // [Note] High-level feature requests that RHI will try to fulfill
    bool bPreferIntegratedGPU = false; // Whether to use iGPU for power saving
    bool bRequireRayTracing = false;
};

class RHI_API IRHIDevice : public FRefCounted {
public:
    IRHIDevice(const FDeviceSpecification& spec);
    virtual ~IRHIDevice();

    // --- Future extensions ---
    // virtual TRef<IRHIBuffer> CreateBuffer(const FRHIBufferDesc& desc) = 0;
    // virtual TRef<IRHICommandContext> GetImmediateContext() = 0;

    /**
     * [Note] Wait for all GPU tasks to complete before destruction
     */
    virtual void WaitIdle() = 0;

    virtual TRef<IRHICommandPool> CreateCommandPool(FCommandPoolSpecification& spec) = 0;

protected:
    FDeviceSpecification m_Spec;
};

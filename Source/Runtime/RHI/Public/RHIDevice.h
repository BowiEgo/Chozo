#pragma once

#include "RHIExport.h"
#include "RHIPipeline.h"
#include "Ref.h"

class IRHIShader;
struct FRHIShaderCreateInfo;

struct FRHIDeviceCreateInfo {
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
    IRHIDevice(const FRHIDeviceCreateInfo& info);
    virtual ~IRHIDevice();

    // --- Shader Resource Factory ---
    virtual TRef<IRHIShader>
        CreateShader(const FRHIShaderCreateInfo& info,
                     const std::vector<uint32_t>* binary) const = 0;

    virtual TRef<IRHIPipeline>
        CreatePipeline(const FRHIPipelineCreateInfo& info) const = 0;

    // --- Future extensions ---
    // virtual TRef<IRHIBuffer> CreateBuffer(const FRHIBufferDesc& desc) = 0;
    // virtual TRef<IRHICommandContext> GetImmediateContext() = 0;

    /**
     * [Note] Wait for all GPU tasks to complete before destruction
     */
    virtual void WaitIdle() = 0;

protected:
    FRHIDeviceCreateInfo m_Info;
};
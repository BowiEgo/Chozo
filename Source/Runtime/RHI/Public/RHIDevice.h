#pragma once

#include "RHIExport.h"
#include "Ref.h"

class IRHIShader;
struct FRHIShaderCreateInfo;

struct FRHIDeviceCreateInfo {
    // --- Debugging ---
    bool bEnableValidationLayers = true;
    bool bEnableGPUProfiling = false;

    // --- Windowing ---
    // [Note] Raw window handle (HWND on Windows, Window on X11)
    void* WindowHandle = nullptr;
    std::vector<const char*> RequiredExtensions;

    // --- Metadata ---
    std::string AppName = "ChozoEngine";
    uint32_t AppVersion = 1;

    // --- Feature Toggles ---
    // [Note] High-level feature requests that RHI will try to fulfill
    bool bPreferIntegratedGPU = false; // Whether to use iGPU for power saving
    bool bRequireRayTracing = false;
};

class RHI_API IRHIDevice : public FRefCounted {
public:
    IRHIDevice(const FRHIDeviceCreateInfo& info) : m_Data(info) {};
    virtual ~IRHIDevice() override = default;

    // --- Shader Resource Factory ---
    virtual TRef<IRHIShader> CreateShader(const FRHIShaderCreateInfo& info) = 0;

    // --- Future extensions ---
    // virtual TRef<IRHIBuffer> CreateBuffer(const FRHIBufferDesc& desc) = 0;
    // virtual TRef<IRHICommandContext> GetImmediateContext() = 0;

    /**
     * [Note] Wait for all GPU tasks to complete before destruction
     */
    virtual void WaitIdle() = 0;

protected:
    FRHIDeviceCreateInfo m_Data;
};
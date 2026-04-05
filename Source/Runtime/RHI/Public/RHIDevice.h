#pragma once

#include "RHIExport.h"
#include "Ref.h"

#include "DescriptorSetCache.h"
#include "ImagePool.h"
#include "RHICommandPool.h"
#include "RHIPipeline.h"
#include "RHISyncObject.h"
#include "SamplerCache.h"
#include "SetLayoutCache.h"

DECLARE_LOG_CATEGORY_EXTERN(LogRHIDevice, Info);

class IRHIContext;
class IRHIResource;
class IRHIImage;
class IRHISampler;
class IRHITexture2D;
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

    virtual TRef<IRHICommandPool> CreateCommandPool(FCommandPoolSpecification& spec)  = 0;
    virtual TRef<IRHIImage> CreateImage(const FImageSpecification& spec)              = 0;
    virtual TRef<IRHISampler> CreateSampler(const FSamplerSpecification& spec)        = 0;
    virtual TRef<IRHISetLayout> CreateSetLayout(const FRHISetLayoutDescription& desc) = 0;
    virtual TRef<IRHITexture2D> CreateTexture2D(const FTextureSpecification& spec)    = 0;
    virtual TRef<IRHIDescriptorSet> CreateDescriptorSet(const FTextureDescriptorInfo& info,
                                                        TRef<IRHISetLayout> setLayout,
                                                        uint32 bindingSlot)           = 0;

    void ReleaseImage(TRef<IRHIImage> image) { return m_ImagePool.ReleaseImage(image); }
    TRef<IRHIImage> GetImage(const FImageSpecification& spec) {
        return m_ImagePool.RequestPersistentImage(spec);
    }
    TRef<IRHISampler> GetSampler(const FSamplerSpecification& spec) {
        return m_SamplerCache.GetOrCreateSampler(spec);
    }
    TRef<IRHISetLayout> GetOrCreateLayout(const FRHISetLayoutDescription& desc) {
        return m_SetLayoutCache.GetOrCreateLayout(desc);
    }
    TRef<IRHISetLayout> GetEmptySetLayout() { return m_SetLayoutCache.GetEmptySetLayout(); }
    TRef<IRHISetLayout> GetStaticSetLayout() { return m_SetLayoutCache.GetStaticSetLayout(); }
    TRef<IRHIDescriptorSet> GetOrCreateDescriptorSet(const FTextureDescriptorInfo& info,
                                                     TRef<IRHISetLayout> setLayout,
                                                     uint32 bindingSlot) {
        return m_DescriptorSetCache.GetOrCreateDescriptorSet(info, setLayout, bindingSlot);
    }

    // Enqueue a cleanup function to be called after it's safe to delete the resource (e.g., after
    // GPU is done with it)
    void EnqueueCleanup(std::function<void()>&& func);
    void TickDeferredDeletion(uint32 currentFrame);
    std::vector<TRef<IRHISetLayout>>
        CreateDescriptorSetLayout(const FRHIPipelineLayoutDescription& desc);

    void Shutdown() {
        m_SamplerCache.Clear();
        m_SetLayoutCache.Clear();
        m_ImagePool.Clear();
        m_DescriptorSetCache.Clear();
    }

protected:
    FDeviceSpecification m_Spec;

    const IRHIContext* m_Context;
    std::vector<DeferredCleanup> m_DeletionQueue;

    CImagePool m_ImagePool;
    CSamplerCache m_SamplerCache;
    CSetLayoutCache m_SetLayoutCache;
    CDescriptorSetCache m_DescriptorSetCache;
};

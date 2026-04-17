#pragma once

#include "RHIExport.h"
#include "RHIResource.h"
#include "RHISampler.h"
#include "RHISetLayout.h"
#include "Ref.h"

DECLARE_LOG_CATEGORY_EXTERN(LogRHIDescriptorSet, Info);

struct FDescriptorBinding {
    uint32_t Binding;
    EUniformType Type;
    IRHIResource* Resource; // IRHIBuffer (Type == UniformBuffer or StorageBuffer) or IRHITexture
                            // (Type == CombinedImageSampler)
    IRHISampler* Sampler;   // Optional, can be null if texture has its own sampler
    EImageLayout ImageLayout = EImageLayout::ShaderReadOnlyOptimal;

    FDescriptorBinding(uint32_t binding, EUniformType type, IRHIResource* resource,
                       IRHISampler* sampler = nullptr,
                       EImageLayout layout  = EImageLayout::ShaderReadOnlyOptimal)
        : Binding(binding), Type(type), Resource(resource), Sampler(sampler), ImageLayout(layout) {}
};

class RHI_API IRHIDescriptorSet : public IRHIResource {
public:
    IRHIDescriptorSet(const WeakRef<IRHIDevice> device, TRef<IRHISetLayout> setLayout,
                      const std::vector<FDescriptorBinding>& bindings);
    virtual ~IRHIDescriptorSet();

    virtual void* GetRawHandle() const = 0;

protected:
    TRef<IRHISetLayout> m_SetLayout;
    std::vector<FDescriptorBinding> m_ResourceBindings;
};
#pragma once

#include "RHIExport.h"
#include "RHIResource.h"
#include "RHITypes.h"

#include "Ref.h"

DECLARE_LOG_CATEGORY_EXTERN(LogRHISampler, Info);

struct FSamplerSpecification {
    EFilter MinFilter         = EFilter::Linear;
    EFilter MagFilter         = EFilter::Linear;
    EAddressMode AddressModeU = EAddressMode::Repeat;
    EAddressMode AddressModeV = EAddressMode::Repeat;
    EAddressMode AddressModeW = EAddressMode::Repeat;
    EMipmapMode MipmapMode    = EMipmapMode::Linear;
    float LodBias             = 0.0f;
    float MaxAnisotropy       = 1.0f;
    bool bEnableAnisotropy    = false;

    static FSamplerSpecification LinearClamp() {
        FSamplerSpecification spec;
        spec.AddressModeU = EAddressMode::ClampToEdge;
        spec.AddressModeV = EAddressMode::ClampToEdge;
        spec.AddressModeW = EAddressMode::ClampToEdge;
        return spec;
    }

    static FSamplerSpecification PointClamp() {
        FSamplerSpecification spec;
        spec.MinFilter    = EFilter::Nearest;
        spec.MagFilter    = EFilter::Nearest;
        spec.AddressModeU = EAddressMode::ClampToEdge;
        spec.AddressModeV = EAddressMode::ClampToEdge;
        spec.AddressModeW = EAddressMode::ClampToEdge;
        return spec;
    }

    bool operator==(const FSamplerSpecification& other) const {
        return MinFilter == other.MinFilter && MagFilter == other.MagFilter &&
               AddressModeU == other.AddressModeU && AddressModeV == other.AddressModeV &&
               AddressModeW == other.AddressModeW && MipmapMode == other.MipmapMode &&
               LodBias == other.LodBias && MaxAnisotropy == other.MaxAnisotropy &&
               bEnableAnisotropy == other.bEnableAnisotropy;
    }
};

namespace std {
template <> struct hash<FSamplerSpecification> {
    size_t operator()(const FSamplerSpecification& spec) const {
        size_t h = 0;
        HashCombine(h, static_cast<size_t>(spec.MinFilter));
        HashCombine(h, static_cast<size_t>(spec.MagFilter));
        HashCombine(h, static_cast<size_t>(spec.AddressModeU));
        HashCombine(h, static_cast<size_t>(spec.AddressModeV));
        HashCombine(h, static_cast<size_t>(spec.AddressModeW));
        HashCombine(h, static_cast<size_t>(spec.MipmapMode));
        HashCombine(h, std::hash<float>{}(spec.LodBias));
        HashCombine(h, std::hash<float>{}(spec.MaxAnisotropy));
        HashCombine(h, std::hash<bool>{}(spec.bEnableAnisotropy));
        return h;
    }
};
} // namespace std

class RHI_API IRHISampler : public IRHIResource {
public:
    IRHISampler(const WeakRef<IRHIDevice> device, const FSamplerSpecification& spec);
    virtual ~IRHISampler();

    virtual EResourceType GetResourceType() const { return EResourceType::Sampler; }

protected:
    FSamplerSpecification m_Spec;
};
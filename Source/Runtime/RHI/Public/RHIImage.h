#pragma once

#include "Buffer.h"
#include "RHIExport.h"
#include "RHIResource.h"
#include "RHITypes.h"
#include "Ref.h"

DECLARE_LOG_CATEGORY_EXTERN(LogRHIImage, Info);

static size_t CombinePhysicalImageTraits(const FExtent2D& size, uint32_t depth, uint32_t mipLevels,
                                         uint32_t layers, EPixelFormat format, EImageUsage usage) {
    size_t h = 0;
    HashCombine(h, std::hash<uint32_t>{}(size.Width));
    HashCombine(h, std::hash<uint32_t>{}(size.Height));
    HashCombine(h, std::hash<uint32_t>{}(depth));
    HashCombine(h, std::hash<uint32_t>{}(mipLevels));
    HashCombine(h, std::hash<uint32_t>{}(layers));
    HashCombine(h, std::hash<uint32_t>{}(static_cast<uint32_t>(format)));
    HashCombine(h, std::hash<uint32_t>{}(
                       static_cast<uint32_t>(usage))); // TODO: Allow compatible usage flags (e.g.,
                                                       // Texture can be compatible with Attachment)
    return h;
}

struct FImageSpecification {
    FExtent2D Size{ 1, 1 };
    uint32_t Depth      = 1;
    uint32_t MipLevels  = 1;
    uint32_t Layers     = 1; // 1 for regular 2D, 6 for Cubemap
    EPixelFormat Format = EPixelFormat::RGBA8_UNORM;
    EImageUsage Usage   = EImageUsage::Sampled | EImageUsage::ColorAttachment;

    bool operator==(const FImageSpecification& other) const {
        return Size.Width == other.Size.Width && Size.Height == other.Size.Height &&
               Depth == other.Depth && MipLevels == other.MipLevels && Layers == other.Layers &&
               Format == other.Format && Usage == other.Usage;
    }
    bool operator!=(const FImageSpecification& other) const { return !(*this == other); }

    size_t GetHash() const {
        return CombinePhysicalImageTraits(Size, Depth, MipLevels, Layers, Format, Usage);
    }
};

struct FImageViewSpecification {
    EImageViewType ViewType  = EImageViewType::View2D;
    EShaderDataFormat Format = EShaderDataFormat::None;

    uint32 BaseMipLevel   = 0;
    uint32 MipCount       = 1;
    uint32 BaseArrayLayer = 0;
    uint32 LayerCount     = 1;

    bool IsFullRange(uint32 totalMips, uint32 totalLayers) const {
        return BaseMipLevel == 0 && (MipCount == totalMips || MipCount == 0) &&
               BaseArrayLayer == 0 && (LayerCount == totalLayers || LayerCount == 0);
    }

    bool operator==(const FImageViewSpecification& other) const {
        return ViewType == other.ViewType && Format == other.Format &&
               BaseMipLevel == other.BaseMipLevel && MipCount == other.MipCount &&
               BaseArrayLayer == other.BaseArrayLayer && LayerCount == other.LayerCount;
    }
};

namespace std {
template <> struct hash<FImageViewSpecification> {
    size_t operator()(const FImageViewSpecification& spec) const {
        size_t h = 0;
        HashCombine(h, std::hash<uint32_t>{}(static_cast<uint32_t>(spec.ViewType)));
        HashCombine(h, std::hash<uint32_t>{}(static_cast<uint32_t>(spec.Format)));
        HashCombine(h, spec.BaseMipLevel);
        HashCombine(h, spec.MipCount);
        HashCombine(h, spec.BaseArrayLayer);
        HashCombine(h, spec.LayerCount);
        return h;
    }
};
} // namespace std

class RHI_API IRHIImage : public IRHIResource {
public:
    IRHIImage(const WeakRef<IRHIDevice> device, const FImageSpecification& spec);
    virtual ~IRHIImage();

    virtual void Destroy()              = 0;
    virtual void SetData(FBuffer& data) = 0;

    virtual const FImageSpecification& GetSpec() const { return m_Spec; }

protected:
    FImageSpecification m_Spec;
};
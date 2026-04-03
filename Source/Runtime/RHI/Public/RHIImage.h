#pragma once

#include "Buffer.h"
#include "RHIExport.h"
#include "RHIResource.h"
#include "RHITypes.h"
#include "Ref.h"

DECLARE_LOG_CATEGORY_EXTERN(LogRHIImage, Info);

namespace FHashTool {
inline size_t CombinePhysicalImageTraits(const FExtent2D& size, uint32_t depth, uint32_t mipLevels,
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
} // namespace FHashTool

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
        return FHashTool::CombinePhysicalImageTraits(Size, Depth, MipLevels, Layers, Format, Usage);
    }
};

class RHI_API IRHIImage : public IRHIResource {
public:
    IRHIImage(const WeakRef<IRHIDevice> device, const FImageSpecification& spec);
    virtual ~IRHIImage();

    virtual const FImageSpecification& GetSpec() const { return m_Spec; }

protected:
    FImageSpecification m_Spec;
};
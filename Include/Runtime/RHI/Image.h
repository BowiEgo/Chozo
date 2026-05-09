#pragma once

#include <Core/Header/Handle.h>
#include <Core/Header/Macros.h>
#include <Core/Memory/Memory.h>
#include <Runtime/RHI/Device.h>
#include <Runtime/RHI/GraphicContext.h>
#include <Runtime/RHI/RHITypes.h>
#include <Runtime/RHI/RHIUtils.h>

namespace CZ {

struct ImageViewSpecification {
    ImageViewType ViewType = ImageViewType::View2D;
    ShaderDataType Format  = ShaderDataType::None;

    uint32 BaseMipLevel   = 0;
    uint32 MipCount       = 1;
    uint32 BaseArrayLayer = 0;
    uint32 LayerCount     = 1;

    bool IsFullRange(uint32 totalMips, uint32 totalLayers) const {
        return BaseMipLevel == 0 && (MipCount == totalMips || MipCount == 0) &&
               BaseArrayLayer == 0 && (LayerCount == totalLayers || LayerCount == 0);
    }

    bool operator==(const ImageViewSpecification& other) const {
        return ViewType == other.ViewType && Format == other.Format &&
               BaseMipLevel == other.BaseMipLevel && MipCount == other.MipCount &&
               BaseArrayLayer == other.BaseArrayLayer && LayerCount == other.LayerCount;
    }
};

static size_t CombinePhysicalImageTraits(const Extent2D& size, uint32_t depth, uint32_t mipLevels,
                                         uint32_t layers, PixelFormat format, ImageUsage usage) {
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

struct ImageSpecification {
    Extent2D Size{ 1, 1 };
    uint32_t Depth     = 1;
    uint32_t MipLevels = 1;
    uint32_t Layers    = 1; // 1 for regular 2D, 6 for Cubemap
    PixelFormat Format = PixelFormat::RGBA8_UNORM;
    ImageUsage Usage   = ImageUsage::Sampled | ImageUsage::ColorAttachment;

    bool operator==(const ImageSpecification& other) const {
        return Size == other.Size && Depth == other.Depth && MipLevels == other.MipLevels &&
               Layers == other.Layers && Format == other.Format && Usage == other.Usage;
    }
    bool operator!=(const ImageSpecification& other) const { return !(*this == other); }

    size_t GetHash() const {
        return CombinePhysicalImageTraits(Size, Depth, MipLevels, Layers, Format, Usage);
    }

    ImageViewSpecification ToImageViewSpec() const {
        ImageViewSpecification viewSpec;
        viewSpec.ViewType   = (Layers == 6) ? ImageViewType::ViewCube : ImageViewType::View2D;
        viewSpec.Format     = RHIUtils::ToShaderDataFormat(Format);
        viewSpec.MipCount   = MipLevels;
        viewSpec.LayerCount = Layers;
        return viewSpec;
    }
};

class ImageObj {
public:
    ImageObj(const Device device, const ImageSpecification& spec)
        : m_Device(device), m_Spec(spec) {}
    virtual ~ImageObj() = default;

protected:
    Device m_Device;
    ImageSpecification m_Spec;

    bool m_IsValid = true;
};

struct Image : Handle<ImageObj> {
    static void Destroy(Image image);
};

} // namespace CZ

namespace std {
template <> struct hash<CZ::ImageViewSpecification> {
    size_t operator()(const CZ::ImageViewSpecification& spec) const {
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
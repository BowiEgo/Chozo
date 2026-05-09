#pragma once

#include <Core/Header/Handle.h>
#include <Core/Memory/Memory.h>
#include <Runtime/RHI/Device.h>
#include <Runtime/RHI/GraphicContext.h>
#include <Runtime/RHI/Image.h>
#include <Runtime/RHI/RHITypes.h>
#include <Runtime/RHI/Sampler.h>

namespace CZ {

inline uint32_t CalculateMipLevels(Extent2D size) {
    uint32_t side = (std::max)(size.Width, size.Height);
    if (side == 0) return 1;

    // Simple version using logarithm, but can be less efficient due to floating-point operations
    // return static_cast<uint32_t>(std::floor(std::log2(side))) + 1;

    // Better version using bitwise operations, more efficient on modern CPUs
    uint32_t levels = 1;
    while (side > 1) {
        side >>= 1;
        levels++;
    }
    return levels;
}

inline ImageUsage ToImageUsage(TextureUsage logicalUsage, PixelFormat format,
                               bool bGenerateMips = false) {
    uint32_t flags = 0;

    if ((uint32_t)logicalUsage & (uint32_t)TextureUsage::Texture) {
        flags |= (uint32_t)ImageUsage::Sampled;
        flags |= (uint32_t)ImageUsage::TransferDst;
    }

    if ((uint32_t)logicalUsage & (uint32_t)TextureUsage::Attachment) {
        if (RHIUtils::IsDepthFormat(format)) {
            flags |= (uint32_t)ImageUsage::DepthStencil;
        } else {
            flags |= (uint32_t)ImageUsage::ColorAttachment;
        }
        flags |= (uint32_t)ImageUsage::Sampled;
    }

    if ((uint32_t)logicalUsage & (uint32_t)TextureUsage::Storage) {
        flags |= (uint32_t)ImageUsage::Storage;
    }

    // Note: If Mip generation is needed, the physical Image must have both Src and Dst permissions
    if (bGenerateMips) {
        flags |= (uint32_t)ImageUsage::TransferSrc;
        flags |= (uint32_t)ImageUsage::TransferDst;
    }

    return static_cast<ImageUsage>(flags);
}

struct TextureSpecification {
    std::string Name;

    Extent2D Size{ 1, 1 };
    PixelFormat Format = PixelFormat::RGBA8_UNORM;
    TextureUsage Usage = TextureUsage::Texture;
    TextureType Type   = TextureType::Texture2D;

    bool bGenerateMips = false;

    SamplerSpecification SamplerSpec;

    ImageSpecification ToImageSpec() const {
        ImageSpecification imgSpec;

        imgSpec.Size = Size;
        imgSpec.Depth =
            1; // Always 1 for 2D textures, can be extended for 3D textures in the future
        imgSpec.MipLevels = bGenerateMips ? CalculateMipLevels(Size) : 1;
        imgSpec.Layers    = (Type == TextureType::TextureCube) ? 6 : 1;
        imgSpec.Format    = Format;
        imgSpec.Usage     = ToImageUsage(Usage, Format, bGenerateMips);
        return imgSpec;
    }

    bool operator==(const TextureSpecification& other) const {
        return Name == other.Name && Size == other.Size && Format == other.Format &&
               Usage == other.Usage && Type == other.Type && bGenerateMips == other.bGenerateMips &&
               SamplerSpec == other.SamplerSpec;
    }
};

class TextureObj {
public:
    TextureObj(const Device device, const TextureSpecification& spec)
        : m_Device(device), m_Spec(spec) {}

    TextureObj(const Device device, const TextureSpecification& spec, Image image)
        : m_Device(device), m_Spec(spec), m_Image(image) {}

    virtual ~TextureObj() = default;

protected:
    Device m_Device;
    TextureSpecification m_Spec;
    Image m_Image;
};

struct Texture : Handle<TextureObj> {
    static void Destroy(Texture Texture);
};

} // namespace CZ

#pragma once

#include "RHIExport.h"

#include "Buffer.h"
#include "Ref.h"

#include "RHIImage.h"
#include "RHIResource.h"
#include "RHISampler.h"
#include "RHISetLayout.h"
#include "RHITypes.h"
#include "RHIUtils.h"

DECLARE_LOG_CATEGORY_EXTERN(LogRHITexture, Info);

inline uint32_t CalculateMipLevels(FExtent2D size) {
    uint32_t side = std::max(size.Width, size.Height);
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

inline EImageUsage ToImageUsage(ETextureUsage logicalUsage, EPixelFormat format,
                                bool bGenerateMips = false) {
    uint32_t flags = 0;

    if ((uint32_t)logicalUsage & (uint32_t)ETextureUsage::Texture) {
        flags |= (uint32_t)EImageUsage::Sampled;
        flags |= (uint32_t)EImageUsage::TransferDst;
    }

    if ((uint32_t)logicalUsage & (uint32_t)ETextureUsage::Attachment) {
        if (ChozoUtils::RHI::IsDepthFormat(format)) {
            flags |= (uint32_t)EImageUsage::DepthStencil;
        } else {
            flags |= (uint32_t)EImageUsage::ColorAttachment;
        }
        flags |= (uint32_t)EImageUsage::Sampled;
    }

    if ((uint32_t)logicalUsage & (uint32_t)ETextureUsage::Storage) {
        flags |= (uint32_t)EImageUsage::Storage;
    }

    // Note: If Mip generation is needed, the physical Image must have both Src and Dst permissions
    if (bGenerateMips) {
        flags |= (uint32_t)EImageUsage::TransferSrc;
        flags |= (uint32_t)EImageUsage::TransferDst;
    }

    return static_cast<EImageUsage>(flags);
}

struct FTextureSpecification {
    std::string Name;

    FExtent2D Size{ 1, 1 };
    EPixelFormat Format = EPixelFormat::RGBA8_UNORM;
    ETextureUsage Usage = ETextureUsage::Texture;
    ETextureType Type   = ETextureType::Texture2D;

    bool bGenerateMips = false;

    FSamplerSpecification SamplerSpec;

    FImageSpecification ToImageSpec() const {
        FImageSpecification imgSpec;

        imgSpec.Size = Size;
        imgSpec.Depth =
            1; // Always 1 for 2D textures, can be extended for 3D textures in the future
        imgSpec.MipLevels = bGenerateMips ? CalculateMipLevels(Size) : 1;
        imgSpec.Layers    = (Type == ETextureType::TextureCube) ? 6 : 1;
        imgSpec.Format    = Format;
        imgSpec.Usage     = ToImageUsage(Usage, Format, bGenerateMips);
        return imgSpec;
    }
};

class RHI_API IRHITexture : public IRHIResource {
public:
    IRHITexture(const WeakRef<IRHIDevice> device, const FTextureSpecification& spec);
    IRHITexture(const WeakRef<IRHIDevice> device, const FTextureSpecification& spec,
                const TRef<IRHIImage> image);
    virtual ~IRHITexture();

    const FTextureSpecification& GetSpec() const { return m_Spec; }
    std::string GetName() const { return m_Spec.Name; }
    FExtent2D GetSize() const { return m_Spec.Size; }
    EPixelFormat GetFormat() const { return m_Spec.Format; }
    ETextureUsage GetUsage() const { return m_Spec.Usage; }

    TRef<IRHIImage> GetImage() const;

    TRef<IRHISampler>
        GetSampler(const FSamplerSpecification spec = FSamplerSpecification::LinearClamp()) const;

    void* GetDescriptorSet(TRef<IRHISetLayout> setLayout = nullptr, uint32_t bindingSlot = 0) const;

protected:
    FTextureSpecification m_Spec;

    TRef<IRHIImage> m_Image = nullptr;
};
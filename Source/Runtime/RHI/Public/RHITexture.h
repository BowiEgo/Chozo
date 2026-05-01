#pragma once

#include "RHIExport.h"

#include "Buffer.h"
#include "Ref.h"
#include "Scope.h"

#include "RHIImage.h"
#include "RHIResource.h"
#include "RHISampler.h"
#include "RHISetLayout.h"
#include "RHITypes.h"
#include "RHIUtils.h"

DECLARE_LOG_CATEGORY_EXTERN(LogRHITexture, Info);

inline uint32_t CalculateMipLevels(FExtent2D size) {
    uint32_t side = (std::max)(size.Width, size.Height);
    if (side == 0) return 1;

    // Simple version using logarithm, but can be less efficient due to floating-point operations
    // return static_cast<uint32_t>(std::floor(std::log2(side))) + 1;

    // std::bit_width(n) 计算表示 n 所需的最小位数
    // 对于 1024 (2^10)，返回 11
    return static_cast<uint32_t>(std::bit_width(side));

    // // Better version using bitwise operations, more efficient on modern CPUs
    // uint32_t levels = 1;
    // while (side > 1) {
    //     side >>= 1;
    //     levels++;
    // }
    // return levels;
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

    bool operator==(const FTextureSpecification& other) const {
        return Name == other.Name && Size == other.Size && Format == other.Format &&
               Usage == other.Usage && Type == other.Type && bGenerateMips == other.bGenerateMips &&
               SamplerSpec == other.SamplerSpec;
    }
};

class RHI_API IRHITexture : public IRHIResource {
public:
    IRHITexture(const WeakRef<IRHIDevice> device, const FTextureSpecification& spec);
    IRHITexture(const WeakRef<IRHIDevice> device, const FTextureSpecification& spec,
                TScope<IRHIImage> ownedImage);
    IRHITexture(const WeakRef<IRHIDevice> device, const FTextureSpecification& spec,
                IRHIImage* borrowedImage);
    IRHITexture(const WeakRef<IRHIDevice> device, const FTextureSpecification& spec,
                FBuffer& data); // TODO: Remove

    virtual ~IRHITexture();
    virtual EResourceType GetResourceType() const { return EResourceType::Texture; }

    const bool IsValid() const {
        auto image = GetImage();
        return image && image->IsValid();
    }
    const FTextureSpecification& GetSpec() const { return m_Spec; }
    std::string GetName() const { return m_Spec.Name; }
    FExtent2D GetSize() const { return m_Spec.Size; }
    EPixelFormat GetFormat() const { return m_Spec.Format; }
    ETextureUsage GetUsage() const { return m_Spec.Usage; }

    IRHIImage* GetImage() const {
        if (m_OwnedImage) {
            return m_OwnedImage.get();
        } else {
            return m_BorrowedImage.GetRawUnsafe();
        }
    }

    TRef<IRHISampler>
        GetSampler(const FSamplerSpecification spec = FSamplerSpecification::LinearClamp()) const;

    void BorrowImage(IRHIImage* borrowedImage) {
        if (m_OwnedImage) return;
        m_BorrowedImage = WeakRef<IRHIImage>(borrowedImage);
    }

protected:
    FTextureSpecification m_Spec;

    TScope<IRHIImage> m_OwnedImage;
    WeakRef<IRHIImage> m_BorrowedImage;
};
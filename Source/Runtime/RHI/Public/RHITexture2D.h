#pragma once

#include "Buffer.h"
#include "RHIExport.h"
#include "RHITypes.h"
#include "Ref.h"

DECLARE_LOG_CATEGORY_EXTERN(LogRHITexture2D, Info);

struct FTexture2DSpecification {
    std::string Name;

    FExtent2D Size{ 1, 1 };
    EPixelFormat Format = EPixelFormat::RGBA8_UNORM;
    ETextureUsage Usage = ETextureUsage::Texture;

    bool bGenerateMips = false;
};

class RHI_API IRHITexture2D : public FRefCounted {
public:
    IRHITexture2D(const FTexture2DSpecification& spec);
    IRHITexture2D(const FTexture2DSpecification& spec, FBuffer& data);
    virtual ~IRHITexture2D();

    virtual void SetData(const FBuffer& data) = 0;

    virtual void* GetDescriptorSet() = 0;

    std::string GetName() const { return m_Spec.Name; }
    FExtent2D GetSize() const { return m_Spec.Size; }
    EPixelFormat GetFormat() const { return m_Spec.Format; }
    ETextureUsage GetUsage() const { return m_Spec.Usage; }

protected:
    FTexture2DSpecification m_Spec;
};
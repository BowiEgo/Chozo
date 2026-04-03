#pragma once

#include "Buffer.h"
#include "RHIExport.h"
#include "RHIResource.h"
#include "RHITypes.h"
#include "Ref.h"

DECLARE_LOG_CATEGORY_EXTERN(LogRHITextureCubemap, Info);

struct FTextureCubemapSpecification {
    std::string Name;

    uint32_t Size = 1;
    EPixelFormat Format = EPixelFormat::RGBA8_UNORM;
    ETextureUsage Usage = ETextureUsage::Texture;

    bool bGenerateMips = false;
    bool bCubeCompatible = true;
};

class RHI_API IRHITextureCubemap : public IRHIResource {
public:
    IRHITextureCubemap(const WeakRef<IRHIDevice> device, const FTextureCubemapSpecification& spec);

    virtual ~IRHITextureCubemap();

    // Set data for 6 faces (usually in order: +X, -X, +Y, -Y, +Z, -Z)
    virtual void SetData(const std::array<FBuffer, 6>& faceBuffers) = 0;
    // or a single buffer version containing all faces data
    virtual void SetData(const FBuffer& buffer) = 0;

    virtual void* GetDescriptorSet() = 0;
    virtual void* GetDescriptorSet() const = 0;

    std::string GetName() const { return m_Spec.Name; }
    uint32_t GetSize() const { return m_Spec.Size; }
    EPixelFormat GetFormat() const { return m_Spec.Format; }
    ETextureUsage GetUsage() const { return m_Spec.Usage; }

protected:
    FTextureCubemapSpecification m_Spec;
};
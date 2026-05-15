#include <Runtime/RHI/RHIAPI.h>
#include <Runtime/RHI/Texture.h>

#include "TextureObj.h"

namespace CZ {

DEFINE_LOG_CATEGORY_STATIC(LogTexture, Info);

DEFINE_HANDLE_DESTROY(TextureObj)

// DEFINE_HANDLE_BACKEND_DESTROY(TextureObj, "vulkan_backend", "DestroyVulkanTextureObj")

std::string Texture::GetName() const { return m_Obj->GetName(); }

TextureType Texture::GetType() const { return m_Obj->GetType(); }

Extent2D Texture::GetSize() const { return m_Obj->GetSize(); }

PixelFormat Texture::GetFormat() const { return m_Obj->GetFormat(); }

TextureUsage Texture::GetUsage() const { return m_Obj->GetUsage(); }

Image Texture::GetImage() { return m_Obj->GetImage(); }

Sampler Texture::GetSampler(const SamplerSpecification spec) {
    return RHIAPI::Get().GetSampler(spec);
}

} // namespace CZ
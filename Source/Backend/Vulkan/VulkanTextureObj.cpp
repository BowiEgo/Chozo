#include "VulkanTextureObj.h"

#include <Core/Header/Assert.h>
#include <Core/Log/LogMacros.h>
#include <Core/Memory/Memory.h>

namespace CZ {

DEFINE_LOG_CATEGORY_STATIC(LogVulkanTexture, Info);

extern "C" {

TextureObj* CreateVulkanTextureObj(const Device device, const TextureSpecification& spec) {
    return New<VulkanTextureObj>(MEMORY_USAGE_RENDER, device, spec);
}

void DestroyVulkanTextureObj(VulkanTextureObj* obj) { Delete(obj); }
}

VulkanTextureObj::VulkanTextureObj(const Device device, const TextureSpecification& spec)
    : TextureObj(device, spec) {}

VulkanTextureObj::VulkanTextureObj(const Device device, const TextureSpecification& spec,
                                   Image image)
    : TextureObj(device, spec, image) {}

VulkanTextureObj::~VulkanTextureObj() {}

} // namespace CZ
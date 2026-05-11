#include "VulkanTextureObj.h"

namespace CZ {

DEFINE_LOG_CATEGORY_STATIC(LogVulkanTexture, Info);

extern "C" {

TextureObj* CreateVulkanTextureObj(const Device device, const TextureSpecification& spec) {
    return CZ_NEW(MEMORY_USAGE_RENDER, VulkanTextureObj, device, spec);
}
}

VulkanTextureObj::VulkanTextureObj(const Device device, const TextureSpecification& spec)
    : TextureObj(device, spec) {}

VulkanTextureObj::VulkanTextureObj(const Device device, const TextureSpecification& spec,
                                   Image image)
    : TextureObj(device, spec, image) {}

VulkanTextureObj::~VulkanTextureObj() {}

} // namespace CZ
#pragma once
#include <cstdint>

#include <Runtime/RHI/Texture.hpp>

#include "VulkanDeviceObj.hpp"

namespace CZ {

class VulkanTextureObj : public TextureObj {
public:
    VulkanTextureObj(const VulkanDeviceObj* device, const TextureSpecification& spec);
    /// Wraps an image created elsewhere (the swapchain); the device is not needed here.
    VulkanTextureObj(const TextureSpecification& spec, Scope<ImageObj> image);

    ~VulkanTextureObj() override;

    VkRenderingAttachmentInfo GetColorAttachmentInfo(const VkClearValue clearColor,
                                                     const bool bClear, uint32_t);
};

} // namespace CZ
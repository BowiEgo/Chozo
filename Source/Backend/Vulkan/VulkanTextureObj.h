#pragma once

#include "../Source/Runtime/RHI/TextureObj.h"

#include "VulkanDeviceObj.h"

namespace CZ {

class VulkanTextureObj : public TextureObj {
public:
    VulkanTextureObj(const VulkanDeviceObj* device, const TextureSpecification& spec);

    VulkanTextureObj(const VulkanDeviceObj* device, const TextureSpecification& spec, Image image);

    ~VulkanTextureObj() override;

    VkRenderingAttachmentInfo GetColorAttachmentInfo(const VkClearValue clearColor,
                                                     const bool bClear, uint32_t);

private:
    const VulkanDeviceObj* m_DeviceObj;
};

} // namespace CZ
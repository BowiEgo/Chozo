#pragma once

#include "VulkanDevice.h"
#include "VulkanRHIExport.h"
#include "VulkanTexture.h"

DECLARE_LOG_CATEGORY_EXTERN(LogVulkanTextureCubemap, Info);

class VULKAN_RHI_API CVulkanTextureCubemap : public CVulkanTexture {
public:
    using CVulkanTexture::CVulkanTexture;
    virtual ~CVulkanTextureCubemap();

    virtual vk::RenderingAttachmentInfo GetColorAttachmentInfo(const vk::ClearValue clearColor,
                                                               const bool bClear, uint32_t face = 0,
                                                               uint32_t mip = 0) override;
};
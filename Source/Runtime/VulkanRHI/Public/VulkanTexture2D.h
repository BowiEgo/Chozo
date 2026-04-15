#pragma once

#include "VulkanDevice.h"
#include "VulkanRHIExport.h"
#include "VulkanTexture.h"

DECLARE_LOG_CATEGORY_EXTERN(LogVulkanTexture2D, Info);

class VULKAN_RHI_API CVulkanTexture2D : public CVulkanTexture {
public:
    using CVulkanTexture::CVulkanTexture;
    virtual ~CVulkanTexture2D();

    virtual vk::RenderingAttachmentInfo GetColorAttachmentInfo(const vk::ClearValue clearColor,
                                                               const bool bClear,
                                                               uint32_t face = 0) override;
};
#pragma once

#include "RHITexture.h"

#include "VulkanDevice.h"
#include "VulkanRHIExport.h"

DECLARE_LOG_CATEGORY_EXTERN(LogVulkanTexture, Info);

class VULKAN_RHI_API CVulkanTexture : public IRHITexture {
public:
    CVulkanTexture(const WeakRef<IRHIDevice> device, const FTextureSpecification& spec);
    CVulkanTexture(const WeakRef<IRHIDevice> device, const FTextureSpecification& spec,
                   TScope<IRHIImage> ownedImage);
    CVulkanTexture(const WeakRef<IRHIDevice> device, const FTextureSpecification& spec,
                   IRHIImage* borrowedImage);
    CVulkanTexture(const WeakRef<IRHIDevice> device, const FTextureSpecification& spec,
                   FBuffer& data); // TODO: Remove

    virtual ~CVulkanTexture();

    virtual vk::RenderingAttachmentInfo GetColorAttachmentInfo(const vk::ClearValue clearColor,
                                                               const bool bClear, uint32_t) = 0;
};
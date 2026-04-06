#pragma once

#include "RHITexture2D.h"

#include "VulkanDevice.h"
#include "VulkanRHIExport.h"

DECLARE_LOG_CATEGORY_EXTERN(LogVulkanTexture2D, Info);

class VULKAN_RHI_API CVulkanTexture2D : public IRHITexture2D {
public:
    CVulkanTexture2D(const WeakRef<IRHIDevice> device, const FTextureSpecification& spec);
    CVulkanTexture2D(const WeakRef<IRHIDevice> device, const FTextureSpecification& spec,
                     const TRef<IRHIImage> image);
    CVulkanTexture2D(const WeakRef<IRHIDevice> device, const FTextureSpecification& spec,
                     FBuffer& data); // TODO: Remove

    virtual ~CVulkanTexture2D();

    vk::RenderingAttachmentInfo GetColorAttachmentInfo(const vk::ClearValue clearColor,
                                                       const bool bClear);

private:
    void Init();
    void Init(FBuffer& data);
    void CreateImageResources();
    void CreateVKImageView();
    void CreateVKSampler();
    vk::DescriptorSet AllocateDescriptorSet(vk::DescriptorSetLayout layout);

private:
    vk::Image m_VKImage;
    vk::ImageView m_VKImageView;
    vk::Format m_VKFormat;
    vk::ImageLayout m_VKCurrentLayout   = vk::ImageLayout::eUndefined;
    vk::Sampler m_VKSampler             = nullptr;
    vk::DeviceMemory m_VKMemory         = nullptr;
    vk::DescriptorSet m_VKDescriptorSet = nullptr;
};
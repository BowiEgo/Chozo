#pragma once

#include "RHITexture2D.h"
#include "VulkanRHIDevice.h"

DECLARE_LOG_CATEGORY_EXTERN(LogVulkanRHITexture2D, Info);

class VULKAN_RHI_API CVulkanRHITexture2D : public IRHITexture2D {
public:
    CVulkanRHITexture2D(const FTextureSpecification& spec, const WeakRef<CVulkanRHIDevice> device,
                        bool bIsOwned = true);
    CVulkanRHITexture2D(const FTextureSpecification& spec, const WeakRef<CVulkanRHIDevice> device,
                        vk::Image image, bool bIsOwned = true); // Wrap an existing VkImage

    virtual ~CVulkanRHITexture2D();

    virtual void* GetDescriptorSet() override { return (void*)GetVKDescriptorSet(); }

    void SetCurrentLayout(vk::ImageLayout layout) { m_VKCurrentLayout = layout; }

    const vk::ImageLayout GetCurrentLayout() const { return m_VKCurrentLayout; }
    const vk::Image GetVKImage() const { return m_VKImage; }
    const vk::ImageView GetVKImageView() const { return m_VKImageView; }
    const vk::Sampler GetVKSampler() const { return m_VKSampler; }
    const vk::Format GetVKFormat() const { return m_VKFormat; }
    vk::DescriptorSet GetVKDescriptorSet();
    vk::RenderingAttachmentInfo GetColorAttachmentInfo(const vk::ClearValue clearColor,
                                                       const bool bClear);

private:
    void Init();
    void CreateImageResources();
    void CreateVKImageView();
    void CreateVKSampler();
    vk::DescriptorSet AllocateDescriptorSet(vk::DescriptorSetLayout layout);

private:
    WeakRef<CVulkanRHIDevice> m_Device;
    bool m_bIsOwned = true; // Whether this texture owns the VkImage (i.e. should destroy it)

    vk::Image m_VKImage;
    vk::ImageView m_VKImageView;
    vk::Format m_VKFormat;
    vk::ImageLayout m_VKCurrentLayout = vk::ImageLayout::eUndefined;
    vk::Sampler m_VKSampler = nullptr;
    vk::DeviceMemory m_VKMemory = nullptr;
    vk::DescriptorSet m_VKDescriptorSet = nullptr;
};
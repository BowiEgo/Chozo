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
                     FBuffer& data);

    virtual ~CVulkanTexture2D();

    virtual void SetData(const FBuffer& data) override;

    // virtual void* GetDescriptorSet() override { return (void*)GetVKDescriptorSet(); }
    // virtual void* GetDescriptorSet() const override { return (void*)GetVKDescriptorSet(); }

    // void SetCurrentLayout(vk::ImageLayout layout) { m_VKCurrentLayout = layout; }

    // const vk::ImageLayout GetCurrentLayout() const { return m_VKCurrentLayout; }
    // const vk::Image GetVKImage() const { return m_VKImage; }
    // const vk::ImageView GetVKImageView() const { return m_VKImageView; }
    // const vk::Sampler GetVKSampler() const { return m_VKSampler; }
    // const vk::Format GetVKFormat() const { return m_VKFormat; }
    // vk::DescriptorSet GetVKDescriptorSet();
    // vk::DescriptorSet GetVKDescriptorSet() const {
    //     return const_cast<CVulkanTexture2D*>(this)->GetVKDescriptorSet();
    //     // FTextureDescriptorInfo info = m_Device->GetDescriptorInfo(m_Spec, m_ImageLayout);
    //     // return m_Device->GetDescriptorSet(info);
    // }
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
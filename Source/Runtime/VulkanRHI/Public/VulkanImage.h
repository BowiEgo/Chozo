#pragma once

#include "RHIImage.h"

#include "VulkanDevice.h"
#include "VulkanRHIExport.h"

#include "vk_mem_alloc.h"

DECLARE_LOG_CATEGORY_EXTERN(LogVulkanImage, Info);

class VULKAN_RHI_API CVulkanImage : public IRHIImage {

public:
    CVulkanImage(const WeakRef<IRHIDevice> device, const FImageSpecification& spec);
    CVulkanImage(const WeakRef<IRHIDevice> device, const FImageSpecification& spec, vk::Image image,
                 bool bIsExternal = false); // Wrap an existing VkImage
    virtual ~CVulkanImage();

    virtual void Destroy() override;
    virtual void SetData(FBuffer& data) override;

    const vk::Image GetVKHandle() const { return m_VKImage; }
    const vk::Format GetVKFormat() const { return m_VKFormat; }
    vk::ImageView GetVKView(const FImageViewSpecification& spec = FImageViewSpecification()) {
        return GetOrCreateVKView(spec);
    }

    const vk::ImageLayout GetCurrentLayout() const { return m_VKCurrentLayout; }
    void SetCurrentLayout(vk::ImageLayout layout) { m_VKCurrentLayout = layout; }

private:
    void Init();
    void CreateImageResources();
    vk::ImageView GetOrCreateVKView(const FImageViewSpecification& spec);

private:
    bool m_bIsExternal = false; // Whether this image owns the VkImage (i.e. should destroy it)

    vk::Format m_VKFormat;
    vk::ImageLayout m_VKCurrentLayout = vk::ImageLayout::eUndefined;
    vk::Image m_VKImage;
    vk::DeviceMemory m_VKMemory = nullptr;
    std::unordered_map<FImageViewSpecification, vk::ImageView> m_ViewCache;

    VmaAllocation m_VmaAllocation = VK_NULL_HANDLE;
};
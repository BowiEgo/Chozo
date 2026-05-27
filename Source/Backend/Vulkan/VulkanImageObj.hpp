#pragma once

#include <Runtime/RHI/Image.hpp>

#ifndef VMA_IMPLEMENTATION
typedef struct VmaAllocation_T* VmaAllocation;
#endif

namespace CZ {

class VulkanDeviceObj;

class VulkanImageObj : public ImageObj {
public:
    VulkanImageObj(const VulkanDeviceObj* deviceObj, const ImageSpecification& spec);
    VulkanImageObj(const VulkanDeviceObj* deviceObj, const ImageSpecification& spec,
                   VkImage vkImage, bool isExternal = false);
    ~VulkanImageObj() override;

    VkImage GetVkImage() { return m_VkImage; }

    VkImageLayout GetVkImageLayout() { return m_VkImageLayout; }

    void SetVkImageLayout(VkImageLayout layout) { m_VkImageLayout = layout; }

    VkImageView GetOrCreateVKView(const ImageViewSpecification& spec = ImageViewSpecification());

private:
    void Init();

    void CreateImageResources();

    const VulkanDeviceObj* m_DeviceObj;

    VmaAllocation m_VmaAllocation = VK_NULL_HANDLE;

    VkFormat m_VkFormat           = VK_FORMAT_UNDEFINED;
    VkImageLayout m_VkImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    VkImage m_VkImage             = VK_NULL_HANDLE;

    std::unordered_map<ImageViewSpecification, VkImageView> m_ViewCache;

    bool m_IsExternal = false; // Whether this image owns the VkImage (i.e. should destroy it)
};

} // namespace CZ
#pragma once

#include <Runtime/RHI/Image.h>

#include "VulkanUtils.h"

namespace CZ {

class VulkanImageObj : public ImageObj {
public:
    VulkanImageObj(const Device device, const ImageSpecification& spec);
    VulkanImageObj(const Device device, const ImageSpecification& spec, VkImage vkImage,
                   bool isExternal = false);
    ~VulkanImageObj() override;

private:
    void Init();
    void CreateImageResources();
    VkImageView GetOrCreateVKView(const ImageViewSpecification& spec);

    VkFormat m_VkFormat             = VK_FORMAT_UNDEFINED;
    VkImageLayout m_VkCurrentLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    VkImage m_VkImage               = VK_NULL_HANDLE;
    VkDeviceMemory m_VkMemory       = VK_NULL_HANDLE;
    std::unordered_map<ImageViewSpecification, VkImageView> m_ViewCache;

    VmaAllocation m_VmaAllocation = VK_NULL_HANDLE;

    bool m_IsExternal = false; // Whether this image owns the VkImage (i.e. should destroy it)
};

} // namespace CZ
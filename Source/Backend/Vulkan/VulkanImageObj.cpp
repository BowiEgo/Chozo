#include "VulkanImageObj.hpp"

#include "VulkanDeviceObj.hpp"

namespace CZ {

VulkanImageObj::VulkanImageObj(const VulkanDeviceObj* deviceObj, const ImageSpecification& spec)
    : ImageObj(spec), m_DeviceObj(deviceObj) {
    Init();
}

VulkanImageObj::VulkanImageObj(const VulkanDeviceObj* deviceObj, const ImageSpecification& spec,
                               VkImage vkImage, bool isExternal)
    : ImageObj(spec), m_DeviceObj(deviceObj), m_VkImage(vkImage), m_IsExternal(isExternal) {
    // Assume the provided image is already in a valid layout and has memory bound.
    // We will query its format and set the current layout to undefined (caller should set it).
    // In a more robust implementation, we might want to allow passing the current layout as well.
    // For now, we'll just set it to undefined and expect the caller to manage it.
    m_VkFormat = VK_FORMAT_UNDEFINED; // Caller should set this if using the wrapper constructor
    m_VkImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
}

VulkanImageObj::~VulkanImageObj() {
    VkDevice logicalDevice = m_DeviceObj->GetLogicalDevice();

    for (auto& [spec, view] : m_ViewCache) {
        vkDestroyImageView(logicalDevice, view, nullptr);
    }

    if (!m_IsExternal) {
        if (m_VmaAllocation != VK_NULL_HANDLE) {
            vmaDestroyImage(m_DeviceObj->GetVmaAllocator(), m_VkImage, m_VmaAllocation);
            m_VkImage       = VK_NULL_HANDLE;
            m_VmaAllocation = VK_NULL_HANDLE;
        } else {
            if (m_VkImage) vkDestroyImage(logicalDevice, m_VkImage, nullptr);
            if (m_VkMemory) vkFreeMemory(logicalDevice, m_VkMemory, nullptr);
        }
    }

    m_IsValid = false;
}

void VulkanImageObj::Init() {
    m_VkFormat = VulkanUtils::ToVkFormat(m_Spec.Format);

    if (!m_IsExternal) {
        CreateImageResources();
    }
}

void VulkanImageObj::CreateImageResources() {
    CZ_CORE_ASSERT(m_DeviceObj, "Device is no longer valid during Image initialization!");

    VkImageCreateInfo imageInfo{};
    imageInfo.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType     = VK_IMAGE_TYPE_2D;
    imageInfo.format        = m_VkFormat;
    imageInfo.extent.width  = m_Spec.Size.Width;
    imageInfo.extent.height = m_Spec.Size.Height;
    imageInfo.extent.depth  = 1;
    imageInfo.mipLevels     = m_Spec.MipLevels;
    imageInfo.arrayLayers   = m_Spec.Layers;
    imageInfo.samples       = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.tiling        = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    if (m_Spec.Layers == 6) {
        imageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
    }

    // Usage flags
    imageInfo.usage = 0;
    if (HasFlag(m_Spec.Usage, ImageUsage::Sampled)) imageInfo.usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
    if (HasFlag(m_Spec.Usage, ImageUsage::ColorAttachment))
        imageInfo.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    if (HasFlag(m_Spec.Usage, ImageUsage::DepthStencil))
        imageInfo.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    if (HasFlag(m_Spec.Usage, ImageUsage::Storage)) imageInfo.usage |= VK_IMAGE_USAGE_STORAGE_BIT;
    if (HasFlag(m_Spec.Usage, ImageUsage::TransferSrc))
        imageInfo.usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    if (HasFlag(m_Spec.Usage, ImageUsage::TransferDst))
        imageInfo.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    if (HasFlag(m_Spec.Usage, ImageUsage::TransientAttachment))
        imageInfo.usage |= VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;

    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage                   = VMA_MEMORY_USAGE_AUTO;
    if (HasFlag(m_Spec.Usage, ImageUsage::ColorAttachment) ||
        HasFlag(m_Spec.Usage, ImageUsage::DepthStencil)) {
        allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
        allocInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
    }
    if (HasFlag(m_Spec.Usage, ImageUsage::TransientAttachment)) {
        allocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
    }

    VkImage vkImageRaw = VK_NULL_HANDLE;
    VkResult result    = vmaCreateImage(m_DeviceObj->GetVmaAllocator(), &imageInfo, &allocInfo,
                                        &vkImageRaw, &m_VmaAllocation, nullptr);
    if (result != VK_SUCCESS) {
        CZ_BACKEND_LOG(Error, "vmaCreateImage failed");
        return;
    }
    m_VkImage  = vkImageRaw;
    m_VkMemory = VK_NULL_HANDLE;
}

VkImageView VulkanImageObj::GetOrCreateVKView(const ImageViewSpecification& spec) {
    if (!m_IsValid) {
        CZ_BACKEND_LOG(Warning, "Attempting to get ImageView for an invalid image!");
        return VK_NULL_HANDLE;
    }
    if (m_ViewCache.contains(spec)) {
        return m_ViewCache[spec];
    }

    CZ_CORE_ASSERT(deviceObj, "Device is no longer valid during ImageView creation!");

    VkDevice logicalDevice = m_DeviceObj->GetLogicalDevice();

    bool isDepth                  = VulkanUtils::IsDepthFormat(m_VkFormat);
    VkFormat vkFormat             = VulkanUtils::ToVkFormat(m_Spec.Format);
    VkImageViewType viewType      = VulkanUtils::ToVkViewType(spec.ViewType);
    VkImageAspectFlags aspectMask = isDepth ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;

    uint32_t mipLevels  = (spec.MipCount == 0) ? m_Spec.MipLevels : spec.MipCount;
    uint32_t layerCount = (spec.LayerCount == 0) ? m_Spec.Layers : spec.LayerCount;

    VkImageSubresourceRange subresourceRange{};
    subresourceRange.aspectMask     = aspectMask;
    subresourceRange.baseMipLevel   = spec.BaseMipLevel;
    subresourceRange.levelCount     = mipLevels;
    subresourceRange.baseArrayLayer = spec.BaseArrayLayer;
    subresourceRange.layerCount     = layerCount;

    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType            = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image            = m_VkImage;
    viewInfo.viewType         = viewType;
    viewInfo.format           = vkFormat;
    viewInfo.subresourceRange = subresourceRange;

    VkImageView view = VK_NULL_HANDLE;
    VkResult result  = vkCreateImageView(logicalDevice, &viewInfo, nullptr, &view);
    if (result != VK_SUCCESS) {
        CZ_BACKEND_LOG(Error, "Failed to create ImageView: VkResult = {}",
                       VulkanUtils::VkResultToString(result));
        return VK_NULL_HANDLE;
    }

    m_ViewCache[spec] = view;
    return view;
}

} // namespace CZ
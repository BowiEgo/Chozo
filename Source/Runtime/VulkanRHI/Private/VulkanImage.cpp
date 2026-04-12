#include "VulkanImage.h"

#include "VulkanCommandBuffer.h"
#include "VulkanDevice.h"
#include "VulkanUtils.h"

#include "vk_mem_alloc.h"

DEFINE_LOG_CATEGORY(LogVulkanImage);

CVulkanImage::CVulkanImage(const WeakRef<IRHIDevice> device, const FImageSpecification& spec)
    : IRHIImage(device, spec) {
    Init();
}

CVulkanImage::CVulkanImage(const WeakRef<IRHIDevice> device, const FImageSpecification& spec,
                           vk::Image image, bool bIsExternal)
    : IRHIImage(device, spec), m_VKImage(image), m_bIsExternal(bIsExternal) {
    CZ_LOG(LogVulkanImage, Trace, "CreateVulkanImage");
    // Assume the provided image is already in a valid layout and has memory bound.
    // We will query its format and set the current layout to undefined (caller should set it).
    // In a more robust implementation, we might want to allow passing the current layout as well.
    // For now, we'll just set it to undefined and expect the caller to manage it.
    m_VKFormat = vk::Format::eUndefined; // Caller should set this if using the wrapper constructor
    m_VKCurrentLayout = vk::ImageLayout::eUndefined;
}

CVulkanImage::~CVulkanImage() {}

void CVulkanImage::Init() {
    m_VKFormat = ChozoUtils::Vulkan::ToVkFormat(m_Spec.Format);

    if (!m_bIsExternal) {
        CreateImageResources();
    }
}

void CVulkanImage::Destroy() {
    // CZ_LOG(LogVulkanImage, Trace, "VulkanImage: destroying...");
    auto device = m_Device.As<CVulkanDevice>();
    if (!device) return;

    vk::Device logicalDevice = device->GetLogicalDevice();

    for (auto& [spec, view] : m_ViewCache) {
        logicalDevice.destroyImageView(view);
    }

    if (!m_bIsExternal) {
        if (m_VmaAllocation != VK_NULL_HANDLE) {
            vmaDestroyImage(device->GetVmaAllocator(), m_VKImage, m_VmaAllocation);
            m_VKImage       = VK_NULL_HANDLE;
            m_VmaAllocation = VK_NULL_HANDLE;
        } else {
            if (m_VKImage) logicalDevice.destroyImage(m_VKImage);
            if (m_VKMemory) logicalDevice.freeMemory(m_VKMemory);
        }
    }
}

void CVulkanImage::SetData(FBuffer& data) {
    auto device = m_Device.lock().As<CVulkanDevice>();
    if (!device) return;

    vk::Device logicalDevice = device->GetLogicalDevice();
    vk::DeviceSize size      = data.Size;

    // Create staging buffer
    vk::Buffer stagingBuffer;
    vk::DeviceMemory stagingMemory;
    device->CreateBuffer(size, vk::BufferUsageFlagBits::eTransferSrc,
                         vk::MemoryPropertyFlagBits::eHostVisible |
                             vk::MemoryPropertyFlagBits::eHostCoherent,
                         stagingBuffer, stagingMemory);

    // Copy to Staging Buffer
    void* mappedData = logicalDevice.mapMemory(stagingMemory, 0, size);
    memcpy(mappedData, data.Data, size);
    logicalDevice.unmapMemory(stagingMemory);

    TRef<CVulkanCommandBuffer> cmdBuffer = device->BeginSingleTimeCommands();
    vk::CommandBuffer vkCmdBuffer        = cmdBuffer->GetVKCommandBuffer();

    // Transition Undefined -> TransferDst
    ChozoUtils::Vulkan::TransitionImageLayout(vkCmdBuffer, m_VKImage, vk::ImageLayout::eUndefined,
                                              vk::ImageLayout::eTransferDstOptimal);

    // Copy Buffer to Image
    vk::BufferImageCopy region;
    region.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
    region.imageSubresource.layerCount = 1;
    region.imageExtent                 = vk::Extent3D(m_Spec.Size.Width, m_Spec.Size.Height, 1);
    vkCmdBuffer.copyBufferToImage(stagingBuffer, m_VKImage, vk::ImageLayout::eTransferDstOptimal,
                                  region);

    // Transition TransferDst -> ShaderReadOnly
    ChozoUtils::Vulkan::TransitionImageLayout(vkCmdBuffer, m_VKImage,
                                              vk::ImageLayout::eTransferDstOptimal,
                                              vk::ImageLayout::eShaderReadOnlyOptimal);

    device->EndSingleTimeCommands(cmdBuffer);

    // Cleanup Staging Resources
    logicalDevice.destroyBuffer(stagingBuffer);
    logicalDevice.freeMemory(stagingMemory);
}

void CVulkanImage::CreateImageResources() {
    auto device = m_Device.lock().As<CVulkanDevice>();
    if (!device) {
        CZ_LOG(LogVulkanImage, Error, "Device is no longer valid during Image resource creation!");
        return;
    }

    vk::Device logicalDevice          = device->GetLogicalDevice();
    vk::PhysicalDevice physicalDevice = device->GetPhysicalDevice();
    bool isDepth                      = ChozoUtils::Vulkan::IsDepthFormat(m_VKFormat);

    vk::ImageCreateInfo imageInfo;
    imageInfo.setImageType(vk::ImageType::e2D)
        .setFormat(m_VKFormat)
        .setExtent({ m_Spec.Size.Width, m_Spec.Size.Height, 1 })
        .setMipLevels(m_Spec.MipLevels)
        .setArrayLayers(m_Spec.Layers)
        .setSamples(vk::SampleCountFlagBits::e1)
        .setTiling(vk::ImageTiling::eOptimal)
        .setInitialLayout(vk::ImageLayout::eUndefined);

    if (m_Spec.Layers == 6) { // Set cube compatible flag for cubemaps
        imageInfo.setFlags(vk::ImageCreateFlagBits::eCubeCompatible);
    }

    // Set correct usage bits based on the specified usage flags. This ensures the image is created
    // with the appropriate capabilities.
    if (HasFlag(m_Spec.Usage, EImageUsage::Sampled))
        imageInfo.usage |= vk::ImageUsageFlagBits::eSampled;
    if (HasFlag(m_Spec.Usage, EImageUsage::ColorAttachment))
        imageInfo.usage |= vk::ImageUsageFlagBits::eColorAttachment;
    if (HasFlag(m_Spec.Usage, EImageUsage::DepthStencil))
        imageInfo.usage |= vk::ImageUsageFlagBits::eDepthStencilAttachment;
    if (HasFlag(m_Spec.Usage, EImageUsage::Storage))
        imageInfo.usage |= vk::ImageUsageFlagBits::eStorage;
    if (HasFlag(m_Spec.Usage, EImageUsage::TransferSrc))
        imageInfo.usage |= vk::ImageUsageFlagBits::eTransferSrc;
    if (HasFlag(m_Spec.Usage, EImageUsage::TransferDst))
        imageInfo.usage |= vk::ImageUsageFlagBits::eTransferDst;
    if (HasFlag(m_Spec.Usage, EImageUsage::TransientAttachment))
        imageInfo.usage |= vk::ImageUsageFlagBits::eTransientAttachment;

    imageInfo.setSharingMode(vk::SharingMode::eExclusive);

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage                   = VMA_MEMORY_USAGE_AUTO;
    if (HasFlag(m_Spec.Usage, EImageUsage::ColorAttachment) ||
        HasFlag(m_Spec.Usage, EImageUsage::DepthStencil)) {
        allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
        allocInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
    }
    if (HasFlag(m_Spec.Usage, EImageUsage::TransientAttachment)) {
        allocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
        // allocInfo.flags |= VMA_ALLOCATION_CREATE_CAN_ALIAS_BIT;
    }

    VkImage vkImageRaw = VK_NULL_HANDLE;
    VkResult result =
        vmaCreateImage(device->GetVmaAllocator(), reinterpret_cast<VkImageCreateInfo*>(&imageInfo),
                       &allocInfo, &vkImageRaw, &m_VmaAllocation, nullptr);
    if (result != VK_SUCCESS) {
        CZ_LOG(LogVulkanImage, Error, "vmaCreateImage failed");
        return;
    }
    m_VKImage  = vkImageRaw;
    m_VKMemory = VK_NULL_HANDLE;

    CZ_LOG(LogVulkanImage, Trace, "Created image via VMA: size={}x{}, layers={}, mips={}",
           m_Spec.Size.Width, m_Spec.Size.Height, m_Spec.Layers, m_Spec.MipLevels);
}

vk::ImageView CVulkanImage::GetOrCreateView(const FImageViewSpecification& spec) {
    if (m_ViewCache.contains(spec)) {
        return m_ViewCache[spec];
    }

    auto device              = m_Device.lock().As<CVulkanDevice>();
    vk::Device logicalDevice = device->GetLogicalDevice();

    // vk::ImageViewCreateInfo viewInfo{};
    // viewInfo.image    = m_VKImage;
    // viewInfo.viewType = ChozoUtils::Vulkan::ToVkViewType(spec.ViewType);

    // // viewInfo.format = (spec.Format == EShaderDataFormat::None)
    // //                       ? ChozoUtils::Vulkan::ToVkFormat(m_Spec.Format)
    // //                       : ChozoUtils::Vulkan::ShaderDataTypeToVkFormat(spec.Format);

    // viewInfo.format = ChozoUtils::Vulkan::ToVkFormat(m_Spec.Format);

    // viewInfo.subresourceRange.aspectMask =
    // ChozoUtils::Vulkan::GetImageAspectFlags(viewInfo.format);
    // viewInfo.subresourceRange.baseMipLevel = spec.BaseMipLevel;
    // viewInfo.subresourceRange.levelCount = (spec.MipCount == 0) ? m_Spec.MipLevels :
    // spec.MipCount; viewInfo.subresourceRange.baseArrayLayer = spec.BaseArrayLayer;
    // viewInfo.subresourceRange.layerCount = (spec.LayerCount == 0) ? m_Spec.Layers :
    // spec.LayerCount;

    vk::Format vkFormat = ChozoUtils::Vulkan::ToVkFormat(m_Spec.Format);
    bool isDepth        = ChozoUtils::Vulkan::IsDepthFormat(vkFormat);

    vk::ImageViewCreateInfo viewInfo;
    viewInfo.setImage(m_VKImage)
        .setViewType(vk::ImageViewType::e2D)
        .setFormat(vkFormat)
        .setSubresourceRange(vk::ImageSubresourceRange(isDepth ? vk::ImageAspectFlagBits::eDepth
                                                               : vk::ImageAspectFlagBits::eColor,
                                                       0, 1, 0, 1));

    vk::ImageView view = logicalDevice.createImageView(viewInfo);

    m_ViewCache[spec] = view;

    return view;
}
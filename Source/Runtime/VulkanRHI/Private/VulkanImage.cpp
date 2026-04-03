#include "VulkanImage.h"

#include "VulkanUtils.h"

DEFINE_LOG_CATEGORY(LogVulkanImage);

CVulkanImage::CVulkanImage(const WeakRef<IRHIDevice> device, const FImageSpecification& spec)
    : IRHIImage(device, spec) {
    Init();
}

CVulkanImage::CVulkanImage(const WeakRef<IRHIDevice> device, const FImageSpecification& spec,
                           vk::Image image, bool bIsOwned)
    : IRHIImage(device, spec), m_VKImage(image), m_bIsOwned(bIsOwned) {
    // Assume the provided image is already in a valid layout and has memory bound.
    // We will query its format and set the current layout to undefined (caller should set it).
    // In a more robust implementation, we might want to allow passing the current layout as well.
    // For now, we'll just set it to undefined and expect the caller to manage it.
    m_VKFormat = vk::Format::eUndefined; // Caller should set this if using the wrapper constructor
    m_VKCurrentLayout = vk::ImageLayout::eUndefined;
}

CVulkanImage::~CVulkanImage() {
    auto device = m_Device.lock().As<CVulkanDevice>();
    if (!device) return;

    vk::Device logicalDevice = device->GetLogicalDevice();

    vk::Image image         = m_VKImage;
    vk::DeviceMemory memory = m_VKMemory;
    bool bOwned             = m_bIsOwned;

    device->EnqueueCleanup([=] {
        if (bOwned) {
            if (image) logicalDevice.destroyImage(image);
            if (memory) logicalDevice.freeMemory(memory);
        }
    });
}

void CVulkanImage::Init() {
    m_VKFormat = ChozoUtils::Vulkan::ToVKFormat(m_Spec.Format);

    if (m_bIsOwned) {
        CreateImageResources();
    }
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
    if ((m_Spec.Usage & EImageUsage::Sampled) == EImageUsage::Sampled) {
        imageInfo.usage |= vk::ImageUsageFlagBits::eSampled;
    }
    if ((m_Spec.Usage & EImageUsage::ColorAttachment) == EImageUsage::ColorAttachment) {
        imageInfo.usage |= vk::ImageUsageFlagBits::eColorAttachment;
    }
    if ((m_Spec.Usage & EImageUsage::DepthStencil) == EImageUsage::DepthStencil) {
        imageInfo.usage |= vk::ImageUsageFlagBits::eDepthStencilAttachment;
    }
    if ((m_Spec.Usage & EImageUsage::Storage) == EImageUsage::Storage) {
        imageInfo.usage |= vk::ImageUsageFlagBits::eStorage;
    }

    imageInfo.setSharingMode(vk::SharingMode::eExclusive);

    m_VKImage = logicalDevice.createImage(imageInfo);
    if (!m_VKImage) {
        CZ_LOG(LogVulkanImage, Error, "Failed to create Vulkan Image");
        return;
    }

    vk::MemoryRequirements memRequirements = logicalDevice.getImageMemoryRequirements(m_VKImage);
    vk::MemoryAllocateInfo allocInfo;
    allocInfo.setAllocationSize(memRequirements.size)
        .setMemoryTypeIndex(device->FindMemoryType(memRequirements.memoryTypeBits,
                                                   vk::MemoryPropertyFlagBits::eDeviceLocal));

    m_VKMemory = logicalDevice.allocateMemory(allocInfo);
    logicalDevice.bindImageMemory(m_VKImage, m_VKMemory, 0);
}

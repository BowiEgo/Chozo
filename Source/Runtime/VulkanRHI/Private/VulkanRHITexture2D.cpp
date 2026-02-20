#include "VulkanRHITexture2D.h"

#include "VulkanUtils.h"

DEFINE_LOG_CATEGORY(LogVulkanRHITexture2D);

CVulkanRHITexture2D::CVulkanRHITexture2D(const FTextureSpecification& spec,
                                         const WeakRef<CVulkanRHIDevice> device, bool bIsOwned)
    : IRHITexture2D(spec), m_Device(device), m_bIsOwned(bIsOwned) {
    Init();
}

CVulkanRHITexture2D::CVulkanRHITexture2D(const FTextureSpecification& spec,
                                         const WeakRef<CVulkanRHIDevice> device, vk::Image image,
                                         bool bIsOwned)
    : IRHITexture2D(spec), m_Device(device), m_VKImage(image), m_bIsOwned(bIsOwned) {
    Init();
}

CVulkanRHITexture2D::~CVulkanRHITexture2D() {
    auto device = m_Device.lock();
    if (!device) return;

    vk::Device logicalDevice = device->GetLogicalDevice();

    if (m_VKImageView) {
        logicalDevice.destroyImageView(m_VKImageView);
        m_VKImageView = nullptr;
    }

    if (m_VKSampler) {
        logicalDevice.destroySampler(m_VKSampler);
        m_VKSampler = nullptr;
    }

    if (m_bIsOwned) {
        if (m_VKImage) {
            logicalDevice.destroyImage(m_VKImage);
        }
        if (m_VKMemory) {
            logicalDevice.freeMemory(m_VKMemory);
        }
    }
}

void CVulkanRHITexture2D::Init() {
    m_VKFormat = ChozoUtils::Vulkan::ToVKFormat(m_Spec.Format);

    if (m_bIsOwned) {
        CreateImageResources();
    }

    if (m_VKImage) {
        CreateVKImageView();
        CreateVKSampler();
    } else {
        CZ_LOG(LogVulkanRHITexture2D, Error, "Cannot create ImageView: m_VKImage is null!");
    }
}

void CVulkanRHITexture2D::CreateImageResources() {
    auto device = m_Device.lock();
    if (!device) {
        CZ_LOG(LogVulkanRHITexture2D, Error,
               "Device is no longer valid during Image resource creation!");
        return;
    }

    vk::Device logicalDevice = device->GetLogicalDevice();
    vk::PhysicalDevice physicalDevice = device->GetPhysicalDevice();
    bool isDepth = ChozoUtils::Vulkan::IsDepthFormat(m_VKFormat);

    vk::ImageCreateInfo imageInfo;
    imageInfo.setImageType(vk::ImageType::e2D)
        .setFormat(m_VKFormat)
        .setExtent({ m_Spec.Size.Width, m_Spec.Size.Height, 1 })
        .setMipLevels(1)
        .setArrayLayers(1)
        .setSamples(vk::SampleCountFlagBits::e1)
        .setTiling(vk::ImageTiling::eOptimal)
        .setInitialLayout(vk::ImageLayout::eUndefined)
        .setUsage(vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst)
        .setSharingMode(vk::SharingMode::eExclusive)
        .setSamples(vk::SampleCountFlagBits::e1);

    // Set correct attachment bit based on format. Depth formats need the depthStencil bit, while
    // color formats need the colorAttachment bit.
    if (isDepth) {
        imageInfo.usage |= vk::ImageUsageFlagBits::eDepthStencilAttachment;
    } else {
        imageInfo.usage |= vk::ImageUsageFlagBits::eColorAttachment;
    }

    m_VKImage = logicalDevice.createImage(imageInfo);

    vk::MemoryRequirements memRequirements = logicalDevice.getImageMemoryRequirements(m_VKImage);

    vk::MemoryAllocateInfo allocInfo;
    allocInfo.setAllocationSize(memRequirements.size)
        .setMemoryTypeIndex(device->FindMemoryType(memRequirements.memoryTypeBits,
                                                   vk::MemoryPropertyFlagBits::eDeviceLocal));

    m_VKMemory = logicalDevice.allocateMemory(allocInfo);
    logicalDevice.bindImageMemory(m_VKImage, m_VKMemory, 0);
}

void CVulkanRHITexture2D::CreateVKImageView() {
    auto device = m_Device.lock();
    if (!device) {
        CZ_LOG(LogVulkanRHITexture2D, Error,
               "Device is no longer valid during ImageView creation!");
        return;
    }

    vk::Device logicalDevice = device->GetLogicalDevice();
    bool isDepth = ChozoUtils::Vulkan::IsDepthFormat(m_VKFormat);

    vk::ImageViewCreateInfo viewInfo;
    viewInfo.setImage(m_VKImage)
        .setViewType(vk::ImageViewType::e2D)
        .setFormat(m_VKFormat)
        .setSubresourceRange(vk::ImageSubresourceRange(isDepth ? vk::ImageAspectFlagBits::eDepth
                                                               : vk::ImageAspectFlagBits::eColor,
                                                       0, 1, 0, 1));

    m_VKImageView = logicalDevice.createImageView(viewInfo);

    CZ_CORE_ASSERT(m_VKImageView, "ImageView creation failed!");
}

void CVulkanRHITexture2D::CreateVKSampler() {
    auto device = m_Device.lock();
    if (!device) {
        CZ_LOG(LogVulkanRHITexture2D, Error, "Device is no longer valid during Sampler creation!");
        return;
    }

    vk::SamplerCreateInfo samplerInfo;
    samplerInfo.setMagFilter(vk::Filter::eLinear)
        .setMinFilter(vk::Filter::eLinear)
        .setAddressModeU(vk::SamplerAddressMode::eRepeat)
        .setAddressModeV(vk::SamplerAddressMode::eRepeat)
        .setAddressModeW(vk::SamplerAddressMode::eRepeat)
        .setAnisotropyEnable(vk::False)
        .setMaxAnisotropy(1.0f)
        .setBorderColor(vk::BorderColor::eIntOpaqueBlack)
        .setUnnormalizedCoordinates(vk::False)
        .setCompareEnable(vk::False)
        .setCompareOp(vk::CompareOp::eAlways)
        .setMipmapMode(vk::SamplerMipmapMode::eLinear)
        .setMipLodBias(0.0f)
        .setMinLod(0.0f)
        .setMaxLod(1.0f);

    // Use the raw logical device to create the sampler to avoid RAII lifetime issues.
    vk::Device logicalDevice = device->GetLogicalDevice();
    m_VKSampler = logicalDevice.createSampler(samplerInfo);

    CZ_CORE_ASSERT(m_VKSampler, "Failed to create texture sampler!");
}

vk::RenderingAttachmentInfo
    CVulkanRHITexture2D::GetColorAttachmentInfo(const vk::ClearValue clearColor,
                                                const bool bClear) {
    return vk::RenderingAttachmentInfo()
        .setImageView(m_VKImageView)
        .setImageLayout(vk::ImageLayout::eColorAttachmentOptimal)
        .setLoadOp(bClear ? vk::AttachmentLoadOp::eClear : vk::AttachmentLoadOp::eLoad)
        .setStoreOp(vk::AttachmentStoreOp::eStore)
        .setClearValue(clearColor);
}

vk::DescriptorSet CVulkanRHITexture2D::GetVKDescriptorSet() {
    // Return immediately if already allocated and updated.
    if (m_VKDescriptorSet) return m_VKDescriptorSet;

    auto device = m_Device.lock();
    if (!device) {
        CZ_LOG(LogVulkanRHITexture2D, Error,
               "Device is no longer valid during DescriptorSet retrieval!");
        return nullptr;
    }

    vk::Device logicalDevice = device->GetLogicalDevice();

    // 1. Get the layout (e.g., from a global Layout cache in the Device).
    vk::DescriptorSetLayout layout =
        device->GetDescriptorSetLayout(EDescriptorLayoutType::CombinedImageSampler);

    // 2. Perform the allocation we defined above.
    m_VKDescriptorSet = AllocateDescriptorSet(layout);
    if (!m_VKDescriptorSet) return nullptr;

    // 3. Update the descriptor set to point to this texture's ImageView.
    vk::DescriptorImageInfo imageInfo;
    imageInfo.setSampler(m_VKSampler)
        .setImageView(m_VKImageView)
        .setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal);

    vk::WriteDescriptorSet descriptorWrite;
    descriptorWrite.setDstSet(m_VKDescriptorSet)
        .setDstBinding(0)
        .setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
        .setDescriptorCount(1)
        .setPImageInfo(&imageInfo);

    logicalDevice.updateDescriptorSets(descriptorWrite, nullptr);

    return m_VKDescriptorSet;
}

vk::DescriptorSet CVulkanRHITexture2D::AllocateDescriptorSet(vk::DescriptorSetLayout layout) {
    auto device = m_Device.lock();
    if (!device) {
        CZ_LOG(LogVulkanRHITexture2D, Error,
               "Device is no longer valid during DescriptorSet creation!");
        return nullptr;
    }

    vk::Device logicalDevice = device->GetLogicalDevice();
    vk::DescriptorPool pool = device->GetGlobalDescriptorPool();

    vk::DescriptorSetAllocateInfo allocInfo;
    allocInfo.setDescriptorPool(pool).setDescriptorSetCount(1).setPSetLayouts(&layout);

    vk::DescriptorSet newSet;

    vk::Result result = logicalDevice.allocateDescriptorSets(&allocInfo, &newSet);
    if (result != vk::Result::eSuccess) {
        CZ_LOG(LogVulkanRHITexture2D, Error, "Failed to allocate DescriptorSet: {}",
               vk::to_string(result));
        return nullptr;
    }

    return newSet;
}

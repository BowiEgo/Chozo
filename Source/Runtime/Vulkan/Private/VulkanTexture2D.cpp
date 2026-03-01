#include "VulkanTexture2D.h"

#include "VulkanCommandBuffer.h"
#include "VulkanUtils.h"

DEFINE_LOG_CATEGORY(LogVulkanTexture2D);

CVulkanTexture2D::CVulkanTexture2D(const FTexture2DSpecification& spec,
                                   const WeakRef<CVulkanDevice> device, bool bIsOwned)
    : IRHITexture2D(spec), m_Device(device), m_bIsOwned(bIsOwned) {
    Init();
}

CVulkanTexture2D::CVulkanTexture2D(const FTexture2DSpecification& spec,
                                   const WeakRef<CVulkanDevice> device, vk::Image image,
                                   bool bIsOwned)
    : IRHITexture2D(spec), m_Device(device), m_VKImage(image), m_bIsOwned(bIsOwned) {
    Init();
}

CVulkanTexture2D::CVulkanTexture2D(const FTexture2DSpecification& spec,
                                   const WeakRef<CVulkanDevice> device, FBuffer& data,
                                   bool bIsOwned)
    : IRHITexture2D(spec), m_Device(device), m_bIsOwned(bIsOwned) {
    Init(data);
}

CVulkanTexture2D::~CVulkanTexture2D() {
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

void CVulkanTexture2D::SetData(const FBuffer& data) {
    auto device = m_Device.lock();
    if (!device) return;

    vk::Device logicalDevice = device->GetLogicalDevice();
    vk::DeviceSize size = data.Size;

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
    vk::CommandBuffer vkCmdBuffer = cmdBuffer->GetVKCommandBuffer();

    // Transition Undefined -> TransferDst
    ChozoUtils::Vulkan::TransitionTextureLayout(vkCmdBuffer, m_VKImage, vk::ImageLayout::eUndefined,
                                                vk::ImageLayout::eTransferDstOptimal);

    // Copy Buffer to Image
    vk::BufferImageCopy region;
    region.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
    region.imageSubresource.layerCount = 1;
    region.imageExtent = vk::Extent3D(m_Spec.Size.Width, m_Spec.Size.Height, 1);
    vkCmdBuffer.copyBufferToImage(stagingBuffer, m_VKImage, vk::ImageLayout::eTransferDstOptimal,
                                  region);

    // Transition TransferDst -> ShaderReadOnly
    ChozoUtils::Vulkan::TransitionTextureLayout(vkCmdBuffer, m_VKImage,
                                                vk::ImageLayout::eTransferDstOptimal,
                                                vk::ImageLayout::eShaderReadOnlyOptimal);

    device->EndSingleTimeCommands(cmdBuffer);

    // Cleanup Staging Resources
    logicalDevice.destroyBuffer(stagingBuffer);
    logicalDevice.freeMemory(stagingMemory);
}

void CVulkanTexture2D::Init() {
    m_VKFormat = ChozoUtils::Vulkan::ToVKFormat(m_Spec.Format);

    if (m_bIsOwned) {
        CreateImageResources();
    }

    if (m_VKImage) {
        CreateVKImageView();
        CreateVKSampler();
    } else {
        CZ_LOG(LogVulkanTexture2D, Error, "Cannot create ImageView: m_VKImage is null!");
    }
}

void CVulkanTexture2D::Init(FBuffer& data) {
    m_VKFormat = ChozoUtils::Vulkan::ToVKFormat(m_Spec.Format);

    if (m_bIsOwned) CreateImageResources();

    if (data) SetData(data);

    if (m_VKImage) {
        CreateVKImageView();
        CreateVKSampler();
    } else {
        CZ_LOG(LogVulkanTexture2D, Error, "Cannot create ImageView: m_VKImage is null!");
    }
}

void CVulkanTexture2D::CreateImageResources() {
    auto device = m_Device.lock();
    if (!device) {
        CZ_LOG(LogVulkanTexture2D, Error,
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

void CVulkanTexture2D::CreateVKImageView() {
    auto device = m_Device.lock();
    if (!device) {
        CZ_LOG(LogVulkanTexture2D, Error, "Device is no longer valid during ImageView creation!");
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

void CVulkanTexture2D::CreateVKSampler() {
    auto device = m_Device.lock();
    if (!device) {
        CZ_LOG(LogVulkanTexture2D, Error, "Device is no longer valid during Sampler creation!");
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
    CVulkanTexture2D::GetColorAttachmentInfo(const vk::ClearValue clearColor, const bool bClear) {
    return vk::RenderingAttachmentInfo()
        .setImageView(m_VKImageView)
        .setImageLayout(vk::ImageLayout::eColorAttachmentOptimal)
        .setLoadOp(bClear ? vk::AttachmentLoadOp::eClear : vk::AttachmentLoadOp::eLoad)
        .setStoreOp(vk::AttachmentStoreOp::eStore)
        .setClearValue(clearColor);
}

vk::DescriptorSet CVulkanTexture2D::GetVKDescriptorSet() {
    // Return immediately if already allocated and updated.
    if (m_VKDescriptorSet) return m_VKDescriptorSet;

    auto device = m_Device.lock();
    if (!device) {
        CZ_LOG(LogVulkanTexture2D, Error,
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

vk::DescriptorSet CVulkanTexture2D::AllocateDescriptorSet(vk::DescriptorSetLayout layout) {
    auto device = m_Device.lock();
    if (!device) {
        CZ_LOG(LogVulkanTexture2D, Error,
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
        CZ_LOG(LogVulkanTexture2D, Error, "Failed to allocate DescriptorSet: {}",
               vk::to_string(result));
        return nullptr;
    }

    return newSet;
}

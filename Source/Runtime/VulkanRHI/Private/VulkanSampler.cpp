#include "VulkanSampler.h"

#include "VulkanDevice.h"
#include "VulkanUtils.h"

CVulkanSampler::CVulkanSampler(const WeakRef<IRHIDevice> device, const FSamplerSpecification& spec)
    : IRHISampler(device, spec) {
    CreateVKSampler();
}

CVulkanSampler::~CVulkanSampler() {
    auto device = m_Device.lock().As<CVulkanDevice>();
    if (!device) return;

    vk::Device logicalDevice = device->GetLogicalDevice();
    vk::Sampler sampler      = m_VKSampler;

    device->EnqueueCleanup([=] {
        if (sampler) logicalDevice.destroySampler(sampler);
    });
}

void CVulkanSampler::CreateVKSampler() {
    auto device = m_Device.lock().As<CVulkanDevice>();
    if (!device) {
        CZ_LOG(LogVulkanSampler, Error, "Device is no longer valid during Sampler creation!");
        return;
    }

    vk::SamplerCreateInfo samplerInfo{};
    samplerInfo.setMagFilter(ChozoUtils::Vulkan::ToVKFilter(m_Spec.MagFilter))
        .setMinFilter(ChozoUtils::Vulkan::ToVKFilter(m_Spec.MinFilter))
        .setAddressModeU(ChozoUtils::Vulkan::ToVKAddressMode(m_Spec.AddressModeU))
        .setAddressModeV(ChozoUtils::Vulkan::ToVKAddressMode(m_Spec.AddressModeU))
        .setAddressModeW(ChozoUtils::Vulkan::ToVKAddressMode(m_Spec.AddressModeU))
        .setAnisotropyEnable(vk::False)
        .setMaxAnisotropy(1.0f)
        .setBorderColor(vk::BorderColor::eIntOpaqueBlack)
        .setUnnormalizedCoordinates(vk::False)
        .setCompareEnable(vk::False)
        .setCompareOp(vk::CompareOp::eAlways)
        .setMipmapMode(ChozoUtils::Vulkan::ToVKMipmapMode(m_Spec.MipmapMode))
        .setMipLodBias(0.0f)
        .setMinLod(0.0f)
        .setMaxLod(1.0f);

    // Use the raw logical device to create the sampler to avoid RAII lifetime issues.
    vk::Device logicalDevice = device->GetLogicalDevice();
    m_VKSampler              = logicalDevice.createSampler(samplerInfo);

    CZ_CORE_ASSERT(m_VKSampler, "Failed to create texture sampler!");
}
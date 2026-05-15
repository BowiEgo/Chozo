#include "VulkanSamplerObj.h"

#include "Runtime/RHI/Sampler.h"
#include "VulkanDeviceObj.h"
#include "VulkanUtils.h"

namespace CZ {

DEFINE_LOG_CATEGORY_STATIC(LogVulkanSampler, Info);

VulkanSamplerObj::VulkanSamplerObj(const VulkanDeviceObj* deviceObj,
                                   const SamplerSpecification& spec)
    : SamplerObj(spec), m_DeviceObj(deviceObj) {
    Init();
}

VulkanSamplerObj::~VulkanSamplerObj() {
    VkDevice logicalDevice = m_DeviceObj->GetLogicalDevice();

    if (m_VkSampler != VK_NULL_HANDLE) {
        vkDestroySampler(logicalDevice, m_VkSampler, nullptr);
    }
}

void VulkanSamplerObj::Init() {
    if (!m_DeviceObj) {
        CZ_LOG(LogVulkanSampler, Error, "Device is no longer valid during Sampler creation!");
        return;
    }

    VkDevice logicalDevice = m_DeviceObj->GetLogicalDevice();

    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType                   = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter               = VulkanUtils::ToVKFilter(m_Spec.MagFilter);
    samplerInfo.minFilter               = VulkanUtils::ToVKFilter(m_Spec.MinFilter);
    samplerInfo.addressModeU            = VulkanUtils::ToVKAddressMode(m_Spec.AddressModeU);
    samplerInfo.addressModeV            = VulkanUtils::ToVKAddressMode(m_Spec.AddressModeV);
    samplerInfo.addressModeW            = VulkanUtils::ToVKAddressMode(m_Spec.AddressModeW);
    samplerInfo.anisotropyEnable        = VK_FALSE;
    samplerInfo.maxAnisotropy           = 1.0f;
    samplerInfo.borderColor             = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable           = VK_FALSE;
    samplerInfo.compareOp               = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode              = VulkanUtils::ToVKMipmapMode(m_Spec.MipmapMode);
    samplerInfo.mipLodBias              = 0.0f;
    samplerInfo.minLod                  = 0.0f;
    samplerInfo.maxLod                  = 1.0f;

    VkSampler vkSampler = VK_NULL_HANDLE;
    VkResult result     = vkCreateSampler(logicalDevice, &samplerInfo, nullptr, &vkSampler);
    if (result != VK_SUCCESS) {
        CZ_LOG(LogVulkanSampler, Error, "Failed to create sampler: %s",
               VulkanUtils::VkResultToString(result));
        return;
    }
    m_VkSampler = vkSampler;

    CZ_CORE_ASSERT(m_VkSampler != VK_NULL_HANDLE, "Failed to create texture sampler!");
}

} // namespace CZ
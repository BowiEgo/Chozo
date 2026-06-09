#include "VulkanDescriptorSetObj.hpp"
#include "Core/Log/LogMacros.hpp"
#include "VulkanDeviceObj.hpp"
#include "VulkanGraphicsBufferObj.hpp"
#include "VulkanImageObj.hpp"
#include "VulkanSamplerObj.hpp"
#include "VulkanSetLayoutObj.hpp"
#include "VulkanTextureObj.hpp"
#include <vulkan/vulkan_core.h>

namespace CZ {

VulkanDescriptorSetObj::VulkanDescriptorSetObj(const VulkanDeviceObj* deviceObj,
                                               SetLayout setLayout,
                                               std::vector<DescriptorBinding>& bindings)
    : DescriptorSetObj(setLayout, bindings), m_DeviceObj(deviceObj) {
    Init();
}

VulkanDescriptorSetObj::~VulkanDescriptorSetObj() {
    VkDevice logicalDevice = m_DeviceObj->GetLogicalDevice();

    vkFreeDescriptorSets(logicalDevice, m_DeviceObj->GetGlobalDescriptorPool(), 1,
                         &m_VkDescriptorSet);
}

VkResult VulkanDescriptorSetObj::Init() {
    VkDevice logicalDevice = m_DeviceObj->GetLogicalDevice();

    VkDescriptorSetLayout layout = m_SetLayout.As<VulkanSetLayoutObj>()->GetVkSetLayout();
    if (layout == VK_NULL_HANDLE) return VK_ERROR_INITIALIZATION_FAILED;

    auto descriptorSet = m_DeviceObj->AllocateSetFromPool(layout);

    if (!descriptorSet) return VK_ERROR_INITIALIZATION_FAILED;

    const uint32_t bindingCount = m_ResourceBindings.size();
    VkWriteDescriptorSet* writes =
        (VkWriteDescriptorSet*)malloc(bindingCount * sizeof(VkWriteDescriptorSet));
    VkDescriptorBufferInfo* bufferInfos =
        (VkDescriptorBufferInfo*)malloc(bindingCount * sizeof(VkDescriptorBufferInfo));
    VkDescriptorImageInfo* imageInfos =
        (VkDescriptorImageInfo*)malloc(bindingCount * sizeof(VkDescriptorImageInfo));

    if (!writes || !bufferInfos || !imageInfos) {
        free(writes);
        free(bufferInfos);
        free(imageInfos);
        vkFreeDescriptorSets(logicalDevice, m_DeviceObj->GetGlobalDescriptorPool(), 1,
                             &descriptorSet);
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    uint32_t bufferInfoCount = 0;
    uint32_t imageInfoCount  = 0;

    for (uint32_t i = 0; i < bindingCount; ++i) {
        const auto binding      = m_ResourceBindings[i];
        VkWriteDescriptorSet* w = &writes[i];
        memset(w, 0, sizeof(VkWriteDescriptorSet));
        w->sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        w->dstSet          = descriptorSet;
        w->dstBinding      = binding.m_Binding;
        w->descriptorCount = 1;

        if (binding.m_Type == ResourceType::GraphicsBuffer) {
            if (binding.m_Buffer) {
                VkDescriptorBufferInfo* info = &bufferInfos[bufferInfoCount++];
                info->buffer = binding.m_Buffer.As<VulkanGraphicsBufferObj>()->GetVKBuffer();
                info->offset = 0;
                info->range  = binding.m_Buffer->GetSize();

                w->descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                w->pBufferInfo    = info;
            }

        } else if (binding.m_Type == ResourceType::Texture) {
            if (binding.m_Texture) {
                auto image                  = binding.m_Texture->GetImage().As<VulkanImageObj>();
                VkDescriptorImageInfo* info = &imageInfos[imageInfoCount++];
                info->sampler     = binding.m_Sampler.As<VulkanSamplerObj>()->GetVkSampler();
                info->imageView   = image->GetOrCreateVKView();
                info->imageLayout = image->GetVkImageLayout();

                w->descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                w->pImageInfo     = info;
            }
        }
    }

    vkUpdateDescriptorSets(logicalDevice, bindingCount, writes, 0, NULL);

    m_VkDescriptorSet = descriptorSet;
    free(writes);
    free(bufferInfos);
    free(imageInfos);

    return VK_SUCCESS;
}

} // namespace CZ
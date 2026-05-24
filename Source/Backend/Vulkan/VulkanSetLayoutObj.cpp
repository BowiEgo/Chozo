#include "VulkanSetLayoutObj.hpp"
#include "VulkanDeviceObj.hpp"
#include "VulkanUtils.hpp"

namespace CZ {

VulkanSetLayoutObj::VulkanSetLayoutObj(const VulkanDeviceObj* deviceObj,
                                       const SetLayoutDescription& desc)
    : m_DeviceObj(deviceObj), m_Desc(desc) {
    Init();
}

VulkanSetLayoutObj::~VulkanSetLayoutObj() {
    CZ_BACKEND_LOG(Trace, "VulkanSetLayout destroying...");
    if (m_VkSetLayout) {
        vkDestroyDescriptorSetLayout(m_DeviceObj->GetLogicalDevice(), m_VkSetLayout, nullptr);
        m_VkSetLayout = VK_NULL_HANDLE;
    }
}

bool VulkanSetLayoutObj::Init() {
    VkDevice logicalDevice = m_DeviceObj->GetLogicalDevice();

    uint32_t bindingCount = m_Desc.Bindings.size();

    VkDescriptorSetLayoutBinding* vkBindings = NULL;
    if (bindingCount > 0) {
        vkBindings = (VkDescriptorSetLayoutBinding*)malloc(bindingCount *
                                                           sizeof(VkDescriptorSetLayoutBinding));
        if (!vkBindings) {
            CZ_BACKEND_LOG(Error, "Failed to allocate memory for layout bindings.");
            return false;
        }

        for (uint32_t i = 0; i < bindingCount; i++) {
            const ShaderResourceBinding* binding = &m_Desc.Bindings[i];
            VkDescriptorSetLayoutBinding* b      = &vkBindings[i];

            b->binding            = binding->Binding;
            b->descriptorType     = VulkanUtils::ToVkDescType(binding->Type);
            b->descriptorCount    = binding->DescriptorCount;
            b->stageFlags         = VulkanUtils::StageToFlagBits(binding->StageFlags);
            b->pImmutableSamplers = NULL;
        }
    }

    VkDescriptorSetLayoutCreateInfo layoutInfo = {
        .sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .pNext        = NULL,
        .flags        = 0,
        .bindingCount = bindingCount,
        .pBindings    = vkBindings
    };

    VkResult result = vkCreateDescriptorSetLayout(logicalDevice, &layoutInfo, NULL, &m_VkSetLayout);

    free(vkBindings);

    RETURN_ON_VULKAN_FAIL(result);

    return true;
}

} // namespace CZ
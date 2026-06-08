#include "VulkanSetLayoutObj.hpp"
#include "VulkanDeviceObj.hpp"
#include "VulkanUtils.hpp"

namespace CZ {

VulkanSetLayoutObj::VulkanSetLayoutObj(const VulkanDeviceObj* deviceObj,
                                       const SetLayoutDescription& desc)
    : m_DeviceObj(deviceObj), m_Desc(desc) {}

VulkanSetLayoutObj::~VulkanSetLayoutObj() {
    CZ_BACKEND_LOG(Trace, "VulkanSetLayout destroying...");
    if (m_VkSetLayout) {
        vkDestroyDescriptorSetLayout(m_DeviceObj->GetLogicalDevice(), m_VkSetLayout, nullptr);
        m_VkSetLayout = VK_NULL_HANDLE;
    }
}

VkResult VulkanSetLayoutObj::Init() {
    VkResult result;

    VkDevice logicalDevice = m_DeviceObj->GetLogicalDevice();
    uint32_t bindingCount  = m_Desc.Bindings.size();

    if (bindingCount == 0) {
        CZ_BACKEND_LOG(Info, "Empty set layout, skipping creation.");
        return VK_SUCCESS;
    }

    VkDescriptorSetLayoutBinding* vkBindings = NULL;
    if (bindingCount > 0) {
        vkBindings = (VkDescriptorSetLayoutBinding*)malloc(bindingCount *
                                                           sizeof(VkDescriptorSetLayoutBinding));
        if (!vkBindings) {
            result = VK_ERROR_INITIALIZATION_FAILED;
            RETURN_WITH_LOG_ON_VULKAN_FAIL(result,
                                           "Failed to allocate memory for layout bindings.");
        }

        for (uint32_t i = 0; i < bindingCount; i++) {
            const ShaderResourceBinding* binding = &m_Desc.Bindings[i];
            VkDescriptorSetLayoutBinding* b      = &vkBindings[i];

            b->binding            = binding->Binding;
            b->descriptorType     = VulkanUtils::ToVkDescType(binding->Type);
            b->descriptorCount    = binding->DescriptorCount;
            b->stageFlags         = VulkanUtils::StageToFlags(binding->StageFlags);
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

    result = vkCreateDescriptorSetLayout(logicalDevice, &layoutInfo, NULL, &m_VkSetLayout);

    free(vkBindings);

    RETURN_WITH_LOG_ON_VULKAN_FAIL(result, "Failed to Create DescriptorSetLayout");

    return result;
}

} // namespace CZ
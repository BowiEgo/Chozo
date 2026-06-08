#pragma once

#include <Runtime/RHI/DescriptorSet.hpp>

namespace CZ {

class VulkanDeviceObj;

class VulkanDescriptorSetObj : public DescriptorSetObj {
public:
    VulkanDescriptorSetObj(const VulkanDeviceObj* deviceObj, SetLayout setLayout,
                           std::vector<DescriptorBinding>& bindings);
    ~VulkanDescriptorSetObj() override;

    static Result<VulkanDescriptorSetObj*, VkResult>
        Create(const VulkanDeviceObj* deviceObj, SetLayout setLayout,
               std::vector<DescriptorBinding>& bindings) {
        if (!deviceObj)
            return Result<VulkanDescriptorSetObj*, VkResult>::Error(VK_ERROR_INITIALIZATION_FAILED);

        auto* obj =
            CZ_NEW(MEMORY_USAGE_RENDER, VulkanDescriptorSetObj, deviceObj, setLayout, bindings);
        if (!obj)
            return Result<VulkanDescriptorSetObj*, VkResult>::Error(VK_ERROR_OUT_OF_HOST_MEMORY);

        VkResult res = obj->Init();
        if (res != VK_SUCCESS) {
            Delete(obj);
            return Result<VulkanDescriptorSetObj*, VkResult>::Error(res);
        }

        return Result<VulkanDescriptorSetObj*, VkResult>::Success(obj);
    }

    void* GetRawHandle() const override { return (void*)GetVkDescriptorSet(); }

    VkDescriptorSet GetVkDescriptorSet() const { return m_VkDescriptorSet; }

private:
    VkResult Init();

    const VulkanDeviceObj* m_DeviceObj;

    VkDescriptorSet m_VkDescriptorSet;
};

} // namespace CZ

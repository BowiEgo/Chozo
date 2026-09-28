#pragma once
#include <vector>

#include <Runtime/RHI/DescriptorSet.hpp>

namespace CZ {

class VulkanDeviceObj;

class VulkanDescriptorSetObj : public DescriptorSetObj {
public:
    VulkanDescriptorSetObj(const VulkanDeviceObj* deviceObj, SetLayout setLayout,
                           std::vector<DescriptorBinding>& bindings);
    ~VulkanDescriptorSetObj() override;

    static Scope<VulkanDescriptorSetObj> Create(const VulkanDeviceObj* deviceObj,
                                                SetLayout setLayout,
                                                std::vector<DescriptorBinding>& bindings) {
        if (!deviceObj) return nullptr;

        auto obj = CZ_CREATE_SCOPE(MEMORY_USAGE_RENDER, VulkanDescriptorSetObj, deviceObj,
                                   setLayout, bindings);

        VkResult res = obj->Init();
        if (res != VK_SUCCESS) return nullptr;

        return obj;
    }

    void* GetRawHandle() const override { return (void*)GetVkDescriptorSet(); }

    VkDescriptorSet GetVkDescriptorSet() const { return m_VkDescriptorSet; }

private:
    VkResult Init();

    const VulkanDeviceObj* m_DeviceObj;

    VkDescriptorSet m_VkDescriptorSet;
};

} // namespace CZ

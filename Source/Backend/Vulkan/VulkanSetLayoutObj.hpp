#pragma once

#include <Runtime/RHI/SetLayout.hpp>

namespace CZ {

class VulkanDeviceObj;

class VulkanSetLayoutObj : public SetLayoutObj {
public:
    VulkanSetLayoutObj(const VulkanDeviceObj* deviceObj, const SetLayoutDescription& desc);
    ~VulkanSetLayoutObj() override;

    static Scope<VulkanSetLayoutObj> Create(VulkanDeviceObj* deviceObj,
                                            const SetLayoutDescription& desc) {
        if (!deviceObj) return nullptr;

        auto obj = CZ_CREATE_SCOPE(MEMORY_USAGE_RENDER, VulkanSetLayoutObj, deviceObj, desc);

        VkResult res = obj->Init();
        if (res != VK_SUCCESS) return nullptr;

        return obj;
    }

    bool Recreate();

    VkDescriptorSetLayout GetVkSetLayout() const { return m_VkSetLayout; }

private:
    VkResult Init();

    const VulkanDeviceObj* m_DeviceObj;
    const SetLayoutDescription m_Desc;

    VkDescriptorSetLayout m_VkSetLayout;
};
} // namespace CZ
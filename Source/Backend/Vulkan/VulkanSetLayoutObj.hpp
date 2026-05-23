#pragma once

#include <Runtime/RHI/SetLayout.hpp>

namespace CZ {

class VulkanDeviceObj;

class VulkanSetLayoutObj : public SetLayoutObj {
public:
    VulkanSetLayoutObj(const VulkanDeviceObj* deviceObj, const SetLayoutDescription& desc);
    ~VulkanSetLayoutObj() override;

    bool Recreate();

    VkDescriptorSetLayout GetVkSetLayout() const { return m_VkSetLayout; }

private:
    bool Init();

    const VulkanDeviceObj* m_DeviceObj;
    const SetLayoutDescription m_Desc;

    VkDescriptorSetLayout m_VkSetLayout;
};
} // namespace CZ
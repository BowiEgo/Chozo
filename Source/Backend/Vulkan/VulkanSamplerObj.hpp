#pragma once

#include "../Source/Runtime/RHI/SamplerObj.hpp"

namespace CZ {

class VulkanDeviceObj;

class VulkanSamplerObj : public SamplerObj {
public:
    VulkanSamplerObj(const VulkanDeviceObj* deviceObj, const SamplerSpecification& spec);
    ~VulkanSamplerObj() override;

    VkSampler GetVkSampler() { return m_VkSampler; }

private:
    void Init();

    const VulkanDeviceObj* m_DeviceObj;

    VkSampler m_VkSampler;
};

} // namespace CZ

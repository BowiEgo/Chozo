#pragma once

#include "../Source/Runtime/RHI/SemaphoreObj.h"

namespace CZ {

class VulkanDeviceObj;

class VulkanSemaphoreObj : public SemaphoreObj {
public:
    VulkanSemaphoreObj(const VulkanDeviceObj* deviceObj);
    ~VulkanSemaphoreObj() override;

    bool Init();
    bool Recreate();

    VkSemaphore GetVkSemaphore() const { return m_Semaphore; }

private:
    const VulkanDeviceObj* m_DeviceObj;

    VkSemaphore m_Semaphore;
};
} // namespace CZ
#include "VulkanRHI.h"

#include "VulkanRHIDevice.h"

extern "C" {

VULKAN_RHI_API IRHIDevice* CreateDevice(const FRHIDeviceCreateInfo& Info) {
    return new CVulkanRHIDevice(Info);
}
}
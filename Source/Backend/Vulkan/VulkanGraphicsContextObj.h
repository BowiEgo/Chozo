#pragma once

#include "../Source/Runtime/RHI/GraphicsContextObj.h"
#include <Runtime/RHI/Device.h>
#include <Runtime/RHI/Swapchain.h>
#include <vulkan/vulkan_core.h>

namespace CZ {

struct VulkanContextWrapper {
    VkInstance Instance;

    VkPhysicalDevice PhysicalDevice;
    VkDevice Device;
    uint32 GraphicsQueueIndex;
    VkQueue GraphicsQueue;
    VkDescriptorPool GlobalDescriptorPool;

    VkSwapchainKHR Swapchain;
};

class VulkanGraphicsContextObj : public GraphicsContextObj {
public:
    VulkanGraphicsContextObj(const GraphicsContextSpecification& spec);
    ~VulkanGraphicsContextObj() override;

    VkInstance GetVKInstance() const { return m_Instance; }
    VkSurfaceKHR GetVKSurface() const { return m_Surface; }

    VulkanContextWrapper GetVulkanContextWrapper();

private:
    void Init();
    void CreateVKInstance();
    void SetupVKDebugMessenger();
    void CreateVKSurface(const void* nativeWindowHandle);

    VkInstance m_Instance                     = VK_NULL_HANDLE;
    VkSurfaceKHR m_Surface                    = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT m_DebugMessenger = VK_NULL_HANDLE;

    PFN_vkCreateDebugUtilsMessengerEXT m_vkCreateDebugUtilsMessengerEXT   = nullptr;
    PFN_vkDestroyDebugUtilsMessengerEXT m_vkDestroyDebugUtilsMessengerEXT = nullptr;
};

} // namespace CZ
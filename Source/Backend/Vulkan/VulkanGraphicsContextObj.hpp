#pragma once

#include <Runtime/RHI/GraphicsContext.hpp>

#include "VulkanAPIObj.hpp"
#include "VulkanDeviceObj.hpp"
#include "VulkanSwapchainObj.hpp"

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
    friend class VulkanAPIObj;
    friend class VulkanSwapchainObj;

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

    // Non-owning views of the scopes owned by `GraphicsContextObj`.
    VulkanDeviceObj* m_DeviceObj       = nullptr;
    VulkanSwapchainObj* m_SwapchainObj = nullptr;
};

} // namespace CZ
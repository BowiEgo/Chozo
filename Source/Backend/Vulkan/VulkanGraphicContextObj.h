#pragma once

#include <Runtime/RHI/GraphicContext.h>

#include <Core/Memory/Memory.h>
#include <Core/Platform/Platform.h>

#ifdef CZ_PLATFORM_MACOS
    #define VK_USE_PLATFORM_METAL_EXT
#endif
#include <vulkan/vulkan.h>

namespace CZ {

class VulkanGraphicContextObj : public GraphicContextObj {
public:
    VulkanGraphicContextObj(const GraphicContextSpecification& spec);
    ~VulkanGraphicContextObj() override;

    VkInstance GetVKInstance() const { return m_Instance; }
    VkSurfaceKHR GetVKSurface() const { return m_Surface; }

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
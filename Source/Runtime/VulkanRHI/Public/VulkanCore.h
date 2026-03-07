#pragma once

#include "CoreMinimal.h"

DECLARE_LOG_CATEGORY_EXTERN(LogVulkan, Info);

class CVulkan {
public:
    CVulkan(const std::vector<const char*>& windowRequiredExtensions,
            const void* nativeWindowHandle);
    ~CVulkan();

private:
    void CreateVKInstance(const std::vector<const char*>& windowRequiredExtensions);
    void SetupVKDebugMessenger();
    void CreateVKSurface(const void* nativeWindowHandle);

public:
    const vk::Instance GetVKInstance() { return *m_Instance; }
    const vk::SurfaceKHR GetVKSurface() { return *m_Surface; }
    const vk::raii::Instance& GetVKRAIIInstance() { return m_Instance; }
    const vk::raii::SurfaceKHR& GetVKRAIISurface() { return m_Surface; }

private:
    // [Note] Vulkan context and instance (Global to the RHI module)
    vk::raii::Context m_Context;
    vk::raii::Instance m_Instance = nullptr;
    vk::raii::DebugUtilsMessengerEXT m_DebugMessenger = nullptr; // Only in Debug
    vk::raii::SurfaceKHR m_Surface = nullptr;
};

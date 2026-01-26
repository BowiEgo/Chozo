#pragma once

#include "Core.h"
#include "Window.h"

#ifndef GLFW_INCLUDE_VULKAN
#define GLFW_INCLUDE_VULKAN
#endif

#if defined(__INTELLISENSE__) || !defined(USE_CPP20_MODULES)
#include <vulkan/vulkan_raii.hpp>
#else
import vulkan_hpp;
#endif

#include <GLFW/glfw3.h>

namespace Chozo {

DECLARE_LOG_CATEGORY_EXTERN(GLFWWindow, Info);

static bool s_GLFWInitialized = false;

static void GLFWErrorCallback(int error, const char *description) {
    CZ_LOG(GLFWWindow, Error, "GLFW Error ({0}):", error, description);
}

class ENGINE_API GLFWWindow : public FWindow {
public:
    GLFWWindow(const FWindowDefinition &windowDef);
    ~GLFWWindow();

    void Shutdown() override;
    void OnUpdate() override;
    void SetVSync(bool enabled) override;
    bool ShouldClose() const override;

private:
    void CreateVKWindow();
    void CreateVKInstance();
    void SetupDebugMessenger();
    void CreateVKSurface();
    void PickPhysicalDevice();
    void CreateLogicalDevice();

    vk::raii::Context m_VkContext;
    vk::raii::Instance m_VkInstance = nullptr;
    vk::raii::DebugUtilsMessengerEXT m_DebugMessenger = nullptr;
    vk::raii::SurfaceKHR m_VkSurface = nullptr;
    vk::raii::PhysicalDevice m_PhysicalDevice = nullptr;
    vk::raii::Device m_LogicalDevice = nullptr;
    vk::raii::Queue m_GraphicsQueue = nullptr;
    vk::raii::Queue m_PresentQueue = nullptr;
    vk::raii::Queue m_ComputeQueue = nullptr;

    GLFWwindow *m_Window{};
};
} // namespace Chozo

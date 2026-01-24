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

namespace Chozo
{

    DECLARE_LOG_CATEGORY_EXTERN(GLFWWindow, Info);

    static bool s_GLFWInitialized = false;

    static void GLFWErrorCallback(int error, const char* description)
    {
        CZ_LOG(GLFWWindow, Error, "GLFW Error ({0}):", error, description);
    }

    class ENGINE_API GLFWWindow : public FWindow
    {
    public:
        GLFWWindow(const FWindowDefinition& windowDef);
        ~GLFWWindow();

        void Shutdown() override;
        void OnUpdate() override;
        void SetVSync(bool enabled) override;
        bool ShouldClose() const override;
    private:
        void CreateVulkanWindow();
        void CreateVulkanSurface();

        vk::raii::Context m_VkContext;
        std::unique_ptr<vk::raii::Instance> m_VkInstance;
        std::unique_ptr<vk::raii::SurfaceKHR> m_VkSurface;

        GLFWwindow* m_Window{};
        GLFWwindow* m_SharedWindow{};
    };
}
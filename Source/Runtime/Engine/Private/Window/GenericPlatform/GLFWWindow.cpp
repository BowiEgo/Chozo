#include "GLFWWindow.h"
#include "VulkanUtils.h"

namespace Chozo
{
    DEFINE_LOG_CATEGORY(GLFWWindow);

    static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData) {
        
        // Log the validation layer message based on its severity
        if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
            CZ_LOG(LogVulkan, Error, "Validation Layer: {0}", pCallbackData->pMessage);
        } else {
            CZ_LOG(LogVulkan, Warning, "Validation Layer: {0}", pCallbackData->pMessage);
        }
        return VK_FALSE; // indicates that the Vulkan call that triggered the validation layer message should not be aborted
    }

    GLFWWindow::GLFWWindow(const FWindowDefinition& windowDef)
        : FWindow(windowDef)
    {
        CreateVulkanWindow();
        CreateVulkanSurface();
        SetupDebugMessenger();
    }

    GLFWWindow::~GLFWWindow()
    {
        Shutdown();
    }

    void GLFWWindow::Shutdown()
    {
        CZ_LOG(GLFWWindow, Trace, "Destroying window {0}", m_Data.Title);

        m_VkSurface.reset();
        m_DebugMessenger.reset();
        m_VkInstance.reset();

        if (m_Window) {
            glfwDestroyWindow(m_Window);
        }

        glfwTerminate();
    }

    void GLFWWindow::OnUpdate()
    {
        /* Poll for and process events */
        glfwPollEvents();
    }

    void GLFWWindow::SetVSync(bool enabled)
    {
        m_Data.VSync = enabled;
    }

    bool GLFWWindow::ShouldClose() const
    {
        return glfwWindowShouldClose(m_Window);
    }

    void GLFWWindow::CreateVulkanWindow()
    {
        CZ_LOG(GLFWWindow, Trace, "Creating window({1}, {2}) for {0}", m_Data.Title, m_Data.Width, m_Data.Height);

        const bool dimensionsInValid = m_Data.Width <= 0 || m_Data.Height <= 0;
        CZ_ASSERT(!dimensionsInValid, "Invalid window dimensions!");

#ifdef CZ_PLATFORM_WIN
        SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
#endif

        // Initialize GLFW window
        if (!s_GLFWInitialized)
        {
            const int success = glfwInit();
            CZ_ASSERT(success, "Could not initialize GLFW!");
            glfwSetErrorCallback(GLFWErrorCallback);
            s_GLFWInitialized = true;
        }

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        m_Window = glfwCreateWindow(m_Data.Width, m_Data.Height, m_Data.Title.c_str(), nullptr, nullptr);

        // Set user pointer to access WindowData in callbacks
        glfwSetWindowUserPointer(m_Window, &m_Data);

        // Pixel ratio
        int windowWidth, windowHeight;
        int fbWidth, fbHeight;
        glfwGetWindowSize(m_Window, &windowWidth, &windowHeight);
        glfwGetFramebufferSize(m_Window, &fbWidth, &fbHeight);
        m_Data.PixelRatio = (float)fbWidth / (float)windowWidth;

		// DPI Scaling
        float xscale, yscale, factor = 1.0f;
#ifdef CZ_PLATFORM_MACOS
        factor = 0.5f;
#endif
        glfwGetWindowContentScale(m_Window, &xscale, &yscale);
        m_Data.XScale = xscale * factor;
        m_Data.YScale = yscale * factor;

        CZ_LOG(GLFWWindow, Info, "GLFW Window created for Vulkan successfully.");

        SetVSync(false);
    }

    void GLFWWindow::CreateVulkanSurface()
    {
        // Check validation layer support
        if (VulkanUtils::EnableValidationLayers && !VulkanUtils::CheckValidationLayerSupport(m_VkContext)) {
            CZ_LOG(GLFWWindow, Warning, "Validation layers requested, but not available!");
        }

        // Get required extensions from GLFW
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
        std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
        if (VulkanUtils::EnableValidationLayers) {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        // Check if the required GLFW extensions are supported by the Vulkan implementation.
        if (!VulkanUtils::CheckInstanceExtensions(m_VkContext, extensions)) {
            throw std::runtime_error("Required Vulkan extensions are not supported!");
        }
                
        // Fill in ApplicationInfo and InstanceCreateInfo
        const vk::ApplicationInfo appInfo = vk::ApplicationInfo()
            .setPApplicationName("Chozo Engine")
            .setApplicationVersion(VK_MAKE_VERSION(1, 0, 0))
            .setPEngineName("Chozo")
            .setEngineVersion(VK_MAKE_VERSION(1, 0, 0))
            .setApiVersion(vk::ApiVersion14);

        vk::InstanceCreateInfo createInfo = vk::InstanceCreateInfo()
            .setPApplicationInfo(&appInfo)
            .setEnabledExtensionCount(static_cast<uint32_t>(extensions.size()))
            .setPpEnabledExtensionNames(extensions.data());

        // Add validation layers if enabled
        if (VulkanUtils::EnableValidationLayers) {
            createInfo.setEnabledLayerCount(static_cast<uint32_t>(VulkanUtils::ValidationLayers.size()))
                      .setPpEnabledLayerNames(VulkanUtils::ValidationLayers.data());
        }
        
        // Create RAII Instance
        try {
            m_VkInstance = std::make_unique<vk::raii::Instance>(m_VkContext, createInfo);
            CZ_LOG(GLFWWindow, Info, "Vulkan RAII Instance created.");

            // Create Surface
            VkSurfaceKHR surfacePtr;
            VkResult result = glfwCreateWindowSurface(**m_VkInstance, m_Window, nullptr, &surfacePtr);

            if (result != VK_SUCCESS) {
                CZ_LOG(GLFWWindow, Fatal, "Failed to create Window Surface!");
            }
            
            // Give the raw pointer to RAII wrapper for automatic management
            m_VkSurface = std::make_unique<vk::raii::SurfaceKHR>(*m_VkInstance, surfacePtr);
            CZ_LOG(GLFWWindow, Info, "Vulkan RAII Surface created.");
        } catch (const vk::SystemError& err) {
            CZ_LOG(GLFWWindow, Fatal, "Vulkan RAII System Error: {0}", err.what());
        } catch (const std::exception& e) {
            CZ_LOG(GLFWWindow, Fatal, "Vulkan RAII Error: {0}", e.what());
        }
    }

    void GLFWWindow::SetupDebugMessenger()
    {
        if (!VulkanUtils::EnableValidationLayers) return;

        vk::DebugUtilsMessageSeverityFlagsEXT severityFlags(
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose | 
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | 
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eError
        );

        vk::DebugUtilsMessageTypeFlagsEXT messageTypeFlags(
            vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | 
            vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance | 
            vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation
        );

        vk::DebugUtilsMessengerCreateInfoEXT messengerInfo = vk::DebugUtilsMessengerCreateInfoEXT()
            .setMessageSeverity(severityFlags)
            .setMessageType(messageTypeFlags)
            .setPfnUserCallback(DebugCallback); // Static method for handling debug messages

        m_DebugMessenger = std::make_unique<vk::raii::DebugUtilsMessengerEXT>(*m_VkInstance, messengerInfo);
    }

}
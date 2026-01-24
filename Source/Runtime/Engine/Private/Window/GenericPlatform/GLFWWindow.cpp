#include "GLFWWindow.h"

namespace Chozo
{
    DEFINE_LOG_CATEGORY(GLFWWindow);

    GLFWWindow::GLFWWindow(const FWindowDefinition& windowDef)
        : FWindow(windowDef)
    {
        CreateVulkanWindow();
        CreateVulkanSurface();
    }

    GLFWWindow::~GLFWWindow()
    {
        Shutdown();
    }

    void GLFWWindow::Shutdown()
    {
        CZ_LOG(GLFWWindow, Trace, "Destroying window {0}", m_Data.Title);
        glfwDestroyWindow(m_Window);
        glfwTerminate();
    }

    void GLFWWindow::OnUpdate()
    {
        /* Poll for and process events */
        while (!glfwWindowShouldClose(m_Window)) {
            glfwPollEvents();
        }
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

#ifdef CZ_PLATFORM_WIN
        SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
#endif

        // Initialize GLFW window
        if (!s_GLFWInitialized)
        {
            const int success = glfwInit();
            CZ_CORE_ASSERT(success, "Could not initialize GLFW!");
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
        // 1. get required extensions from GLFW
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
        
        // 2. fill in ApplicationInfo and InstanceCreateInfo
        const vk::ApplicationInfo appInfo = vk::ApplicationInfo()
            .setPApplicationName("Chozo Engine")
            .setApplicationVersion(VK_MAKE_VERSION(1, 0, 0))
            .setPEngineName("Chozo")
            .setEngineVersion(VK_MAKE_VERSION(1, 0, 0))
            .setApiVersion(vk::ApiVersion14);

        vk::InstanceCreateInfo createInfo = vk::InstanceCreateInfo()
            .setPApplicationInfo(&appInfo)
            .setEnabledExtensionCount(glfwExtensionCount)
            .setPpEnabledExtensionNames(glfwExtensions);
        
        // 3. create RAII Instance
        try {
            m_VkInstance = std::make_unique<vk::raii::Instance>(m_VkContext, createInfo);
            CZ_LOG(GLFWWindow, Info, "Vulkan RAII Instance created.");

            // 4. create Surface
            VkSurfaceKHR surfacePtr;
            VkResult result = glfwCreateWindowSurface(**m_VkInstance, m_Window, nullptr, &surfacePtr);

            if (result != VK_SUCCESS) {
                CZ_LOG(GLFWWindow, Fatal, "Failed to create Window Surface!");
            }
            
            // 5. Give the raw pointer to RAII wrapper for automatic management
            m_VkSurface = std::make_unique<vk::raii::SurfaceKHR>(*m_VkInstance, surfacePtr);
            CZ_LOG(GLFWWindow, Info, "Vulkan RAII Surface created.");
            
        } catch (const std::exception& e) {
            CZ_LOG(GLFWWindow, Fatal, "Vulkan RAII Error: {0}", e.what());
        }
    }

}
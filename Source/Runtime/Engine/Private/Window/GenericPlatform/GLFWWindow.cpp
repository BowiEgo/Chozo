#include "GLFWWindow.h"
#include "VulkanUtils.h"
#include "vulkan/vulkan.hpp"
#include "vulkan/vulkan_raii.hpp"

namespace Chozo {
DEFINE_LOG_CATEGORY(GLFWWindow);

static VKAPI_ATTR VkBool32 VKAPI_CALL
DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
              VkDebugUtilsMessageTypeFlagsEXT messageType,
              const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData) {

    // Log the validation layer message based on its severity
    if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
        CZ_LOG(LogVulkan, Error, "Validation Layer: {0}", pCallbackData->pMessage);
    } else {
        CZ_LOG(LogVulkan, Warning, "Validation Layer: {0}", pCallbackData->pMessage);
    }
    return VK_FALSE; // indicates that the Vulkan call that triggered the
                     // validation layer message should not be aborted
}

GLFWWindow::GLFWWindow(const FWindowDefinition &windowDef) : FWindow(windowDef) {
    CreateVKWindow();
    CreateVKInstance();
    SetupDebugMessenger();
    CreateVKSurface();
    PickPhysicalDevice();
    CreateLogicalDevice();
    CreateSwapchain();
    CreateImageViews();
    CreateGraphicsPipeline();
}

GLFWWindow::~GLFWWindow() { Shutdown(); }

void GLFWWindow::Shutdown() {
    CZ_LOG(GLFWWindow, Trace, "Destroying window {0}", m_Data.Title);

    if (m_Window) {
        glfwDestroyWindow(m_Window);
    }

    glfwTerminate();
}

void GLFWWindow::OnUpdate() {
    /* Poll for and process events */
    glfwPollEvents();
}

void GLFWWindow::SetVSync(bool enabled) { m_Data.VSync = enabled; }

bool GLFWWindow::ShouldClose() const { return glfwWindowShouldClose(m_Window); }

void GLFWWindow::CreateVKWindow() {
    CZ_LOG(GLFWWindow, Trace, "Creating window({1}, {2}) for {0}", m_Data.Title, m_Data.Width,
           m_Data.Height);

    const bool dimensionsInValid = m_Data.Width <= 0 || m_Data.Height <= 0;
    CZ_ASSERT(!dimensionsInValid, "Invalid window dimensions!");

#ifdef CZ_PLATFORM_WIN
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
#endif

    // Initialize GLFW window
    if (!s_GLFWInitialized) {
        const int success = glfwInit();
        CZ_ASSERT(success, "Could not initialize GLFW!");
        glfwSetErrorCallback(GLFWErrorCallback);
        s_GLFWInitialized = true;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    m_Window =
        glfwCreateWindow(m_Data.Width, m_Data.Height, m_Data.Title.c_str(), nullptr, nullptr);

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

void GLFWWindow::CreateVKInstance() {
    // Check validation layer support
    if (VulkanUtils::EnableValidationLayers &&
        !VulkanUtils::CheckValidationLayerSupport(m_VkContext)) {
        CZ_LOG(GLFWWindow, Warning, "Validation layers requested, but not available!");
    }

    // Get required extensions from GLFW
    uint32_t glfwExtensionCount = 0;
    const char **glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    std::vector<const char *> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
    if (VulkanUtils::EnableValidationLayers) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    // Check if the required GLFW extensions are supported by the Vulkan
    // implementation.
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

    vk::InstanceCreateInfo createInfo =
        vk::InstanceCreateInfo()
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
        m_VkInstance = vk::raii::Instance(m_VkContext, createInfo);
        CZ_LOG(GLFWWindow, Info, "Vulkan RAII Instance created.");
    } catch (const vk::SystemError &err) {
        CZ_LOG(GLFWWindow, Fatal, "Vulkan RAII System Error: {0}", err.what());
    } catch (const std::exception &e) {
        CZ_LOG(GLFWWindow, Fatal, "Vulkan RAII Error: {0}", e.what());
    }
}

void GLFWWindow::SetupDebugMessenger() {
    if (!VulkanUtils::EnableValidationLayers)
        return;

    vk::DebugUtilsMessageSeverityFlagsEXT severityFlags(
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eError);

    vk::DebugUtilsMessageTypeFlagsEXT messageTypeFlags(
        vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
        vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
        vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation);

    vk::DebugUtilsMessengerCreateInfoEXT messengerInfo =
        vk::DebugUtilsMessengerCreateInfoEXT()
            .setMessageSeverity(severityFlags)
            .setMessageType(messageTypeFlags)
            .setPfnUserCallback(DebugCallback); // Static method for handling debug messages

    m_DebugMessenger = vk::raii::DebugUtilsMessengerEXT(m_VkInstance, messengerInfo);
}

void GLFWWindow::CreateVKSurface() {
    VkSurfaceKHR surfaceHandle;
    VkResult result = glfwCreateWindowSurface(*m_VkInstance, m_Window, nullptr, &surfaceHandle);

    if (result != VK_SUCCESS) {
        CZ_LOG(GLFWWindow, Fatal, "Failed to create Window Surface!");
    }

    // Give the raw pointer to RAII wrapper for automatic management
    m_VkSurface = vk::raii::SurfaceKHR(m_VkInstance, surfaceHandle);
    CZ_LOG(GLFWWindow, Info, "Vulkan RAII Surface created.");
}

void GLFWWindow::PickPhysicalDevice() {
    auto devices = m_VkInstance.enumeratePhysicalDevices();

    if (devices.empty()) {
        CZ_LOG(GLFWWindow, Fatal, "Failed to find GPUs with Vulkan support");
    }

    for (const auto &device : devices) {
        m_PhysicalDevice = device;
        break;
    }

    auto deviceProperties = m_PhysicalDevice.getProperties();
    auto deviceFeatures = m_PhysicalDevice.getFeatures();

    VulkanUtils::LogPhysicalDeviceInfo(deviceProperties);
    VulkanUtils::LogMemoryBudget(m_PhysicalDevice);
}

void GLFWWindow::CreateLogicalDevice() {
    VulkanUtils::QueueFamilyIndices indices =
        VulkanUtils::FindQueueFamilies(m_PhysicalDevice, m_VkSurface);

    std::set<uint32_t> uniqueQueueFamilies = {indices.Graphics.value(), indices.Present.value(),
                                              indices.Compute.value()};

    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
    float queuePriority = 1.0f; // range is [0.0, 1.0]
    for (uint32_t queueFamily : uniqueQueueFamilies) {
        queueCreateInfos.push_back({{}, queueFamily, 1, &queuePriority});
    }

    // Configure Vulkan 1.3 and Extension features
    // IMPORTANT: Keep these structures in scope until m_LogicalDevice is created
    vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT extendedDynamicStateFeatures;
    extendedDynamicStateFeatures.extendedDynamicState = vk::True;

    vk::PhysicalDeviceVulkan13Features vulkan13Features;
    vulkan13Features.dynamicRendering = vk::True;
    vulkan13Features.pNext = &extendedDynamicStateFeatures;

    // Link features to DeviceCreateInfo
    std::vector<const char *> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    vk::DeviceCreateInfo deviceCreateInfo({}, queueCreateInfos, {}, deviceExtensions);
    deviceCreateInfo.pNext = &vulkan13Features; // Hook the feature chain here

    // Create the logical device
    m_LogicalDevice = vk::raii::Device(m_PhysicalDevice, deviceCreateInfo);

    // Get queue handles for each operation type
    m_GraphicsQueue = vk::raii::Queue(m_LogicalDevice, indices.Graphics.value(), 0);
    m_PresentQueue = vk::raii::Queue(m_LogicalDevice, indices.Present.value(), 0);
    m_ComputeQueue = vk::raii::Queue(m_LogicalDevice, indices.Compute.value(), 0);

    CZ_LOG(GLFWWindow, Info, "Queue Family Indices -> Graphics: {}, Present: {}, Compute: {}",
           indices.Graphics.value(), indices.Present.value(), indices.Compute.value());
}

void GLFWWindow::CreateSwapchain() {
    VulkanUtils::SwapchainSupportDetails details =
        VulkanUtils::QuerySwapchainSupport(m_PhysicalDevice, m_VkSurface);

    int pixelWidth, pixelHeight;
    glfwGetFramebufferSize(m_Window, &pixelWidth, &pixelHeight);
    vk::SurfaceFormatKHR surfaceFormat = VulkanUtils::ChooseSwapSurfaceFormat(details.formats);
    vk::PresentModeKHR presentMode = VulkanUtils::ChooseSwapPresentMode(details.presentModes);
    vk::Extent2D extent =
        VulkanUtils::ChooseSwapExtent(details.capabilities, pixelWidth, pixelHeight);

    // Determine image count (Minimum + 1 for triple buffering)
    uint32_t imageCount = details.capabilities.minImageCount + 1;
    if (details.capabilities.maxImageCount > 0 && imageCount > details.capabilities.maxImageCount) {
        imageCount = details.capabilities.maxImageCount;
    }

    vk::SwapchainCreateInfoKHR createInfo;
    createInfo.surface = *m_VkSurface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;

    // If indices are different, use Concurrent mode; otherwise use Exclusive
    VulkanUtils::QueueFamilyIndices indices =
        VulkanUtils::FindQueueFamilies(m_PhysicalDevice, m_VkSurface);
    uint32_t queueFamilyIndices[] = {indices.Graphics.value(), indices.Present.value()};
    if (indices.Graphics != indices.Present) {
        createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode = vk::SharingMode::eExclusive;
    }

    // Use the current transform of the surface to avoid unintended rotation
    if (details.capabilities.supportedTransforms & vk::SurfaceTransformFlagBitsKHR::eIdentity) {
        createInfo.preTransform = vk::SurfaceTransformFlagBitsKHR::eIdentity;
    } else {
        createInfo.preTransform = details.capabilities.currentTransform;
    }

    createInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
    createInfo.presentMode = presentMode;
    createInfo.clipped = true;
    createInfo.oldSwapchain = nullptr;

    m_Swapchain = vk::raii::SwapchainKHR(m_LogicalDevice, createInfo);

    // Retrieve the images created by the swapchain
    m_SwapchainImages = m_Swapchain.getImages();
    m_SwapchainImageFormat = surfaceFormat.format;
    m_SwapchainExtent = extent;
}

void GLFWWindow::CreateImageViews() {
    m_SwapchainImageViews.clear();

    vk::ImageViewCreateInfo createInfo;
    createInfo.viewType = vk::ImageViewType::e2D;
    createInfo.format = m_SwapchainImageFormat;

    createInfo.components.r = vk::ComponentSwizzle::eIdentity;
    createInfo.components.g = vk::ComponentSwizzle::eIdentity;
    createInfo.components.b = vk::ComponentSwizzle::eIdentity;
    createInfo.components.a = vk::ComponentSwizzle::eIdentity;

    createInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
    createInfo.subresourceRange.baseMipLevel = 0;
    createInfo.subresourceRange.levelCount = 1;
    createInfo.subresourceRange.baseArrayLayer = 0;
    createInfo.subresourceRange.layerCount = 1;

    for (auto image : m_SwapchainImages) {
        createInfo.image = image;
        m_SwapchainImageViews.emplace_back(m_LogicalDevice, createInfo);
    }
}

void GLFWWindow::CreateGraphicsPipeline() {}
} // namespace Chozo

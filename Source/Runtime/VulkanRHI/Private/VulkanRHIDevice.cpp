#include "VulkanRHIDevice.h"

#include "VulkanRHIShader.h"
#include "VulkanUtils.h"

DEFINE_LOG_CATEGORY(LogVulkanRHIDevice);

extern "C" VULKAN_RHI_API IRHIDevice*
    CreateVulkanDevice(const FRHIDeviceCreateInfo& Info) {
    return new CVulkanRHIDevice(Info);
}

static VKAPI_ATTR VkBool32 VKAPI_CALL
    DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                  VkDebugUtilsMessageTypeFlagsEXT messageType,
                  const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                  void* pUserData) {

    // Log the validation layer message based on its severity
    if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
        CZ_LOG(LogVulkanRHIDevice, Error, "Validation Layer: {0}",
               pCallbackData->pMessage);
    } else {
        CZ_LOG(LogVulkanRHIDevice, Warning, "Validation Layer: {0}",
               pCallbackData->pMessage);
    }
    return VK_FALSE; // indicates that the Vulkan call that triggered the
                     // validation layer message should not be aborted
}

CVulkanRHIDevice::CVulkanRHIDevice(const FRHIDeviceCreateInfo& info)
    : IRHIDevice(info) {
    CreateVKInstance();
    SetupDebugMessenger();
    CreateVKSurface();
    PickPhysicalDevice();

    Init();
}

void CVulkanRHIDevice::Init() {
    FQueueFamilyIndices indices =
        ChozoUtils::Vulkan::FindQueueFamilies(m_PhysicalDevice, m_Surface);

    std::set<uint32_t> uniqueQueueFamilies;
    if (indices.Graphics.has_value())
        uniqueQueueFamilies.insert(indices.Graphics.value());
    if (indices.Present.has_value())
        uniqueQueueFamilies.insert(indices.Present.value());
    if (indices.Compute.has_value())
        uniqueQueueFamilies.insert(indices.Compute.value());

    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
    float queuePriority = 1.0f; // range is [0.0, 1.0]
    for (uint32_t queueFamily : uniqueQueueFamilies) {
        queueCreateInfos.push_back({{}, queueFamily, 1, &queuePriority});
    }

    // Enable Dynamic State (Viewport/Scissor/etc.) to avoid pipeline
    // bloat
    vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT
        extendedDynamicStateFeatures;
    extendedDynamicStateFeatures.extendedDynamicState = vk::True;

    // Enable Modern Rendering (Vulkan 1.3)
    vk::PhysicalDeviceVulkan13Features vulkan13Features;
    vulkan13Features.dynamicRendering = vk::True;
    vulkan13Features.pNext = &extendedDynamicStateFeatures;

    // Link features to DeviceCreateInfo
    std::vector<const char*> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    vk::DeviceCreateInfo deviceCreateInfo({}, queueCreateInfos, {},
                                          deviceExtensions);
    deviceCreateInfo.pNext = &vulkan13Features; // Hook the feature chain here

    // Create the logical device
    try {
        m_LogicalDevice = vk::raii::Device(m_PhysicalDevice, deviceCreateInfo);

        // Safely fetch queue handles
        if (indices.Graphics.has_value())
            m_GraphicsQueue =
                vk::raii::Queue(m_LogicalDevice, indices.Graphics.value(), 0);
        if (indices.Present.has_value())
            m_PresentQueue =
                vk::raii::Queue(m_LogicalDevice, indices.Present.value(), 0);
        if (indices.Compute.has_value())
            m_ComputeQueue =
                vk::raii::Queue(m_LogicalDevice, indices.Compute.value(), 0);

        CZ_LOG(LogVulkanRHIDevice, Info,
               "Queue Family Indices -> Graphics: {}, Present: {}, Compute: {}",
               indices.Graphics.value(), indices.Present.value(),
               indices.Compute.value());

    } catch (const std::exception& e) {
        CZ_LOG(LogVulkanRHIDevice, Error, "Vulkan Device Creation Failed: {0}",
               e.what());
    }

    CZ_LOG(LogVulkanRHIDevice, Info, "Vulkan Logical Device Created.");
}

void CVulkanRHIDevice::CreateVKInstance() {
    // Check validation layer support
    if (ChozoUtils::Vulkan::EnableValidationLayers &&
        !ChozoUtils::Vulkan::CheckValidationLayerSupport(m_Context)) {
        CZ_LOG(LogVulkanRHIDevice, Warning,
               "Validation layers requested, but not available!");
    }

    // Get required extensions from GLFW
    auto extensions = m_Data.RequiredExtensions;

    if (ChozoUtils::Vulkan::EnableValidationLayers) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    // Check if the required GLFW extensions are supported by the Vulkan
    // implementation.
    if (!ChozoUtils::Vulkan::CheckInstanceExtensions(m_Context, extensions)) {
        throw std::runtime_error(
            "Required Vulkan extensions are not supported!");
    }

    // Fill in ApplicationInfo and InstanceCreateInfo
    const vk::ApplicationInfo appInfo =
        vk::ApplicationInfo()
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
    if (ChozoUtils::Vulkan::EnableValidationLayers) {
        createInfo
            .setEnabledLayerCount(static_cast<uint32_t>(
                ChozoUtils::Vulkan::ValidationLayers.size()))
            .setPpEnabledLayerNames(
                ChozoUtils::Vulkan::ValidationLayers.data());
    }

    // Create RAII Instance
    try {
        m_Instance = vk::raii::Instance(m_Context, createInfo);
        CZ_LOG(LogVulkanRHIDevice, Info, "Vulkan RAII Instance created.");
    } catch (const vk::SystemError& err) {
        CZ_LOG(LogVulkanRHIDevice, Fatal, "Vulkan RAII System Error: {0}",
               err.what());
    } catch (const std::exception& e) {
        CZ_LOG(LogVulkanRHIDevice, Fatal, "Vulkan RAII Error: {0}", e.what());
    }
}

void CVulkanRHIDevice::SetupDebugMessenger() {
    if (!ChozoUtils::Vulkan::EnableValidationLayers)
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
            .setPfnUserCallback(
                DebugCallback); // Static method for handling debug messages

    m_DebugMessenger =
        vk::raii::DebugUtilsMessengerEXT(m_Instance, messengerInfo);
}

void CVulkanRHIDevice::CreateVKSurface() {
    VkSurfaceKHR surfaceHandle;
    VkResult result;

    auto rawHandle = m_Data.WindowHandle;

    try {
#ifdef CHOZO_PLATFORM_WINDOWS
        /* Using Vulkan-Hpp Win32 structure */
        vk::Win32SurfaceCreateInfoKHR createInfo({}, GetModuleHandle(nullptr),
                                                 (HWND)rawHandle);

        // Directly initialize the RAII wrapper
        m_Surface = vk::raii::SurfaceKHR(m_Instance, createInfo);

#elif defined(CHOZO_PLATFORM_LINUX)
        // Implement Xlib/Wayland logic here...
#elif defined(CHOZO_PLATFORM_MACOS)
        // Implement Metal/Cocoa logic here...
#endif

        CZ_LOG(LogVulkanRHIDevice, Info, "Vulkan RAII Surface created.");
    } catch (const std::exception& e) {
        CZ_LOG(LogVulkanRHIDevice, Fatal,
               "Failed to create Window Surface: {0}", e.what());
    }
}

void CVulkanRHIDevice::PickPhysicalDevice() {
    auto devices = m_Instance.enumeratePhysicalDevices();

    if (devices.empty()) {
        CZ_LOG(LogVulkanRHIDevice, Fatal,
               "Failed to find GPUs with Vulkan support");
    }

    for (const auto& device : devices) {
        m_PhysicalDevice = device;
        break;
    }

    auto deviceProperties = m_PhysicalDevice.getProperties();
    auto deviceFeatures = m_PhysicalDevice.getFeatures();

    ChozoUtils::Vulkan::LogPhysicalDeviceInfo(deviceProperties);
    ChozoUtils::Vulkan::LogMemoryBudget(m_PhysicalDevice);
}

TRef<IRHIShader>
    CVulkanRHIDevice::CreateShader(const FRHIShaderCreateInfo& info) {
    return TRef<CVulkanRHIShader>::Create(info);
}
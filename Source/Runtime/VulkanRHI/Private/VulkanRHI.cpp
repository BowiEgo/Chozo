#include "VulkanRHI.h"
#include "VulkanUtils.h"

DEFINE_LOG_CATEGORY(LogVulkanRHI);

extern "C" {
VULKAN_RHI_API IRHI* CreateVulkanRHI(const FRHICreateInfo& info) {
    return new CVulkanRHI(info);
}
}

static VKAPI_ATTR VkBool32 VKAPI_CALL
    DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                  VkDebugUtilsMessageTypeFlagsEXT messageType,
                  const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                  void* pUserData) {

    // Log the validation layer message based on its severity
    if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
        CZ_LOG(LogVulkanRHI, Error, "Validation Layer: {0}",
               pCallbackData->pMessage);
    } else {
        CZ_LOG(LogVulkanRHI, Warning, "Validation Layer: {0}",
               pCallbackData->pMessage);
    }
    return VK_FALSE; // indicates that the Vulkan call that triggered the
                     // validation layer message should not be aborted
}

CVulkanRHI::CVulkanRHI(const FRHICreateInfo& info) : m_Data(info) { Init(); }

void CVulkanRHI::Init() {
    CreateVKInstance();
    if constexpr (GIsDebug) {
        SetupVKDebugMessenger();
        CZ_LOG(LogVulkan, Info, "Vulkan Debug Messenger enabled.");
    }
    CreateVKSurface();

    FRHIDeviceCreateInfo info;
    info.AppName = "ChozoEngine";
    info.AppVersion = 1;

    m_Device = CreateDevice(info);
}

void CVulkanRHI::CreateVKInstance() {
    // Check validation layer support
    if (ChozoUtils::Vulkan::EnableValidationLayers &&
        !ChozoUtils::Vulkan::CheckValidationLayerSupport(m_Context)) {
        CZ_LOG(LogVulkanRHI, Warning,
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
        CZ_LOG(LogVulkanRHI, Info, "Vulkan RAII Instance created.");
    } catch (const vk::SystemError& err) {
        CZ_LOG(LogVulkanRHI, Fatal, "Vulkan RAII System Error: {0}",
               err.what());
    } catch (const std::exception& e) {
        CZ_LOG(LogVulkanRHI, Fatal, "Vulkan RAII Error: {0}", e.what());
    }
}

void CVulkanRHI::SetupVKDebugMessenger() {
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

void CVulkanRHI::CreateVKSurface() {
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

        CZ_LOG(LogVulkanRHI, Info, "Vulkan RAII Surface created.");
    } catch (const std::exception& e) {
        CZ_LOG(LogVulkanRHI, Fatal, "Failed to create Window Surface: {0}",
               e.what());
    }
}

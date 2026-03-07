#include "VulkanCore.h"

#include "VulkanUtils.h"
#ifdef CZ_PLATFORM_MACOS
    #include "MacUtils.h"
#endif

DEFINE_LOG_CATEGORY(LogVulkan);

static VKAPI_ATTR VkBool32 VKAPI_CALL
    DebugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                  vk::DebugUtilsMessageTypeFlagsEXT messageType,
                  const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {

    // [Note] Access data using pCallbackData->pMessage (vulkan-hpp handles the pointer mapping)
    if (messageSeverity >= vk::DebugUtilsMessageSeverityFlagBitsEXT::eError) {
        CZ_LOG(LogVulkan, Error, "Validation Layer: {0}", pCallbackData->pMessage);
    } else {
        CZ_LOG(LogVulkan, Warning, "Validation Layer: {0}", pCallbackData->pMessage);
    }

    return VK_FALSE;
}

CVulkan::CVulkan(const std::vector<const char*>& windowRequiredExtensions,
                 const void* nativeWindowHandle) {
    CreateVKInstance(windowRequiredExtensions);
    if constexpr (GIsDebug) {
        SetupVKDebugMessenger();
        CZ_LOG(LogVulkan, Info, "Vulkan Debug Messenger enabled.");
    }
    CreateVKSurface(nativeWindowHandle);
}

CVulkan::~CVulkan() { CZ_LOG(LogVulkan, Trace, "Vulkan destroying..."); }

void CVulkan::CreateVKInstance(const std::vector<const char*>& windowRequiredExtensions) {
    // Check validation layer support
    if (ChozoUtils::Vulkan::EnableValidationLayers &&
        !ChozoUtils::Vulkan::CheckValidationLayerSupport(m_Context)) {
        CZ_LOG(LogVulkan, Warning, "Validation layers requested, but not available!");
    }

    // Get required extensions from GLFW
    auto extensions = windowRequiredExtensions;

    if (ChozoUtils::Vulkan::EnableValidationLayers) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

#ifdef CZ_PLATFORM_MACOS
    extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
    extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
#endif

    // Check if the required GLFW extensions are supported by the Vulkan
    // implementation.
    if (!ChozoUtils::Vulkan::CheckInstanceExtensions(m_Context, extensions)) {
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
    if (ChozoUtils::Vulkan::EnableValidationLayers) {
        createInfo
            .setEnabledLayerCount(
                static_cast<uint32_t>(ChozoUtils::Vulkan::ValidationLayers.size()))
            .setPpEnabledLayerNames(ChozoUtils::Vulkan::ValidationLayers.data());
    }

#ifdef CZ_PLATFORM_MACOS
    createInfo.setFlags(vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR);
#endif

    // Create RAII Instance
    try {
        m_Instance = vk::raii::Instance(m_Context, createInfo);
        CZ_LOG(LogVulkan, Info, "Vulkan Instance created.");
    } catch (const vk::SystemError& err) {
        CZ_LOG(LogVulkan, Fatal, "Vulkan System Error: {0}", err.what());
    } catch (const std::exception& e) {
        CZ_LOG(LogVulkan, Fatal, "Vulkan Error: {0}", e.what());
    }
}

void CVulkan::SetupVKDebugMessenger() {
    if (!ChozoUtils::Vulkan::EnableValidationLayers) return;

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

    m_DebugMessenger = vk::raii::DebugUtilsMessengerEXT(m_Instance, messengerInfo);
}

void CVulkan::CreateVKSurface(const void* nativeWindowHandle) {
    VkSurfaceKHR surfaceHandle;
    VkResult result;

    try {
#ifdef CZ_PLATFORM_WINDOWS
        /* Using Vulkan-Hpp Win32 structure */
        vk::Win32SurfaceCreateInfoKHR createInfo({}, GetModuleHandle(nullptr),
                                                 (HWND)nativeWindowHandle);

        // Directly initialize the RAII wrapper
        m_Surface = vk::raii::SurfaceKHR(m_Instance, createInfo);

#elif defined(CZ_PLATFORM_LINUX)
        // Implement Xlib/Wayland logic here...
#elif defined(CZ_PLATFORM_MACOS)
        // Implement Metal/Cocoa logic here...
        auto metalLayer = ChozoUtils::Mac::GetMetalLayerFromNSWindow(nativeWindowHandle);
        vk::MetalSurfaceCreateInfoEXT createInfo;
        createInfo.setPLayer(static_cast<const CAMetalLayer*>(metalLayer));

        try {
            m_Surface = vk::raii::SurfaceKHR(m_Instance, createInfo);
            CZ_LOG(LogVulkan, Info, "Vulkan Metal Surface created successfully.");
        } catch (const std::exception& e) {
            CZ_LOG(LogVulkan, Fatal, "Failed to create Metal Surface: {0}", e.what());
        }
#endif

        CZ_LOG(LogVulkan, Info, "Vulkan Surface created.");
    } catch (const std::exception& e) {
        CZ_LOG(LogVulkan, Fatal, "Failed to create Window Surface: {0}", e.what());
    }
}
#include "VulkanGraphicContextObj.h"

#include "VulkanUtils.h"

namespace CZ {

DEFINE_LOG_CATEGORY_STATIC(LogVulkanGraphicContext, Info);

extern "C" {

GraphicContextObj* CreateVulkanGraphicContextObj(const GraphicContextSpecification& spec) {
    return CZ_NEW(MEMORY_USAGE_RENDER, VulkanGraphicContextObj, spec);
}
}

static VKAPI_ATTR VkBool32 VKAPI_CALL
    DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                  VkDebugUtilsMessageTypeFlagsEXT messageType,
                  const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {

    if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
        CZ_LOG(LogVulkanGraphicContext, Error, "Validation Layer: {0}", pCallbackData->pMessage);
    } else {
        CZ_LOG(LogVulkanGraphicContext, Warning, "Validation Layer: {0}", pCallbackData->pMessage);
    }
    return VK_FALSE;
}

VulkanGraphicContextObj::VulkanGraphicContextObj(const GraphicContextSpecification& spec)
    : GraphicContextObj(spec) {
    Init();
}

VulkanGraphicContextObj::~VulkanGraphicContextObj() {
    if (m_Surface != VK_NULL_HANDLE) {
        vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
    }
    if (m_vkDestroyDebugUtilsMessengerEXT && m_DebugMessenger != VK_NULL_HANDLE) {
        m_vkDestroyDebugUtilsMessengerEXT(m_Instance, m_DebugMessenger, nullptr);
    }
    if (m_Instance != VK_NULL_HANDLE) {
        vkDestroyInstance(m_Instance, nullptr);
    }
}

void VulkanGraphicContextObj::Init() {
    CreateVKInstance();
    SetupVKDebugMessenger();
    CreateVKSurface(m_Spec.NativeWindow);
}

void VulkanGraphicContextObj::CreateVKInstance() {
    VkApplicationInfo appInfo{};
    appInfo.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName   = "Chozo Engine";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName        = "Chozo";
    appInfo.engineVersion      = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion         = VK_API_VERSION_1_4;

    auto extensions = m_Spec.WindowRequiredExtensions;

    if (m_Spec.EnableValidationLayers) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

#ifdef CZ_PLATFORM_MACOS
    extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
    extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
#endif

    uint32_t extCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extCount, nullptr);
    std::vector<VkExtensionProperties> availableExts(extCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extCount, availableExts.data());

    for (const auto* req : extensions) {
        bool found = false;
        for (auto& avail : availableExts) {
            if (strcmp(req, avail.extensionName) == 0) {
                found = true;
                break;
            }
        }
        if (!found) {
            CZ_LOG(LogVulkanGraphicContext, Error, "Missing required extension: {0}", req);
            throw std::runtime_error("Required Vulkan extension not supported");
        }
    }

    VkInstanceCreateInfo createInfo{};
    createInfo.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo        = &appInfo;
    createInfo.enabledExtensionCount   = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();
    createInfo.enabledLayerCount       = 0;

#ifdef CZ_PLATFORM_MACOS
    createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif

    if (m_Spec.EnableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(VulkanUtils::ValidationLayers.size());
        createInfo.ppEnabledLayerNames = VulkanUtils::ValidationLayers.data();
    }

    if (vkCreateInstance(&createInfo, nullptr, &m_Instance) != VK_SUCCESS) {
        CZ_LOG(LogVulkanGraphicContext, Fatal, "Failed to create Vulkan instance.");
        throw std::runtime_error("Vulkan instance creation failed");
    }
    CZ_LOG(LogVulkanGraphicContext, Info, "Vulkan Instance created.");
}

void VulkanGraphicContextObj::SetupVKDebugMessenger() {
    if (!m_Spec.EnableValidationLayers) return;

    m_vkCreateDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
        vkGetInstanceProcAddr(m_Instance, "vkCreateDebugUtilsMessengerEXT"));
    m_vkDestroyDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
        vkGetInstanceProcAddr(m_Instance, "vkDestroyDebugUtilsMessengerEXT"));
    if (!m_vkCreateDebugUtilsMessengerEXT || !m_vkDestroyDebugUtilsMessengerEXT) {
        CZ_LOG(LogVulkanGraphicContext, Warning, "Debug extension not available");
        return;
    }

    VkDebugUtilsMessengerCreateInfoEXT messengerInfo{};
    messengerInfo.sType           = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    messengerInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                    VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                    VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    messengerInfo.messageType     = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                    VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
                                    VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
    messengerInfo.pfnUserCallback = DebugCallback;

    if (m_vkCreateDebugUtilsMessengerEXT(m_Instance, &messengerInfo, nullptr, &m_DebugMessenger) !=
        VK_SUCCESS) {
        CZ_LOG(LogVulkanGraphicContext, Error, "Failed to create debug messenger");
    }

    CZ_LOG(LogVulkanGraphicContext, Info, "Vulkan Debug Messenger enabled.");
}

void VulkanGraphicContextObj::CreateVKSurface(const void* nativeWindowHandle) {
    VkResult result = VK_ERROR_UNKNOWN;

#ifdef CZ_PLATFORM_WINDOWS
    VkWin32SurfaceCreateInfoKHR surfaceInfo{};
    surfaceInfo.sType     = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    surfaceInfo.hinstance = GetModuleHandle(nullptr);
    surfaceInfo.hwnd      = (HWND)nativeWindowHandle;
    result                = vkCreateWin32SurfaceKHR(m_Instance, &surfaceInfo, nullptr, &m_Surface);

#elif defined(CZ_PLATFORM_MACOS)
    auto metalLayer = Platform::Mac::GetMetalLayerFromNSWindow(nativeWindowHandle);
    VkMetalSurfaceCreateInfoEXT surfaceInfo{};
    surfaceInfo.sType  = VK_STRUCTURE_TYPE_METAL_SURFACE_CREATE_INFO_EXT;
    surfaceInfo.pLayer = static_cast<const CAMetalLayer*>(metalLayer);
    result             = vkCreateMetalSurfaceEXT(m_Instance, &surfaceInfo, nullptr, &m_Surface);
#endif

    if (result != VK_SUCCESS) {
        CZ_LOG(LogVulkanGraphicContext, Fatal, "Failed to create window surface");
        throw std::runtime_error("Vulkan surface creation failed");
    }
    CZ_LOG(LogVulkanGraphicContext, Info, "Vulkan Surface created.");
}

} // namespace CZ
#include "VulkanUtils.h"

DEFINE_LOG_CATEGORY(LogVulkanUtils);

namespace ChozoUtils::Vulkan {

vk::ShaderStageFlagBits StageToFlagBits(EShaderStage shaderStage) {
    switch (shaderStage) {
#define GENERATE_CASE(ENUM, LOWER, UPPER, SHORT, GLSL, VULKAN)                                     \
    case EShaderStage::ENUM:                                                                       \
        return vk::ShaderStageFlagBits::e##VULKAN;
        FOREACH_SHADER_STAGE(GENERATE_CASE)
#undef GENERATE_CASE
    default:
        return static_cast<vk::ShaderStageFlagBits>(0);
    }
}

template <typename T, typename Getter>
bool IsSupported(const std::vector<const char*>& required, const std::vector<T>& available,
                 Getter&& nameGetter) {
    return std::ranges::all_of(required, [&](const char* name) {
        return std::ranges::any_of(
            available, [&](const T& item) { return std::string_view(nameGetter(item)) == name; });
    });
}

bool CheckValidationLayerSupport(vk::raii::Context& context) {
    auto available = context.enumerateInstanceLayerProperties();
    return IsSupported(ValidationLayers, available, [](auto& l) { return l.layerName; });
}

bool CheckInstanceExtensions(const vk::raii::Context& context,
                             const std::vector<const char*>& required) {
    auto available = context.enumerateInstanceExtensionProperties();
    return IsSupported(required, available, [](const auto& p) { return p.extensionName; });
}

FQueueFamilyIndices FindQueueFamilies(const vk::raii::PhysicalDevice& physicalDevice,
                                      const vk::raii::SurfaceKHR& surface) {
    FQueueFamilyIndices indices;

    // find the index of the first queue family that supports graphics
    auto queueFamilyProps = physicalDevice.getQueueFamilyProperties();

    for (uint32 i = 0; i < queueFamilyProps.size(); i++) {
        const auto queueFamilyProp = queueFamilyProps[i];

        if (queueFamilyProp.queueFlags & vk::QueueFlagBits::eGraphics) {
            indices.Graphics = i;
        }

        if (physicalDevice.getSurfaceSupportKHR(i, *surface)) {
            indices.Present = i;
        }

        if (queueFamilyProp.queueFlags & vk::QueueFlagBits::eCompute) {
            if (!indices.Compute.has_value() ||
                !(queueFamilyProp.queueFlags & vk::QueueFlagBits::eGraphics)) {
                indices.Compute = i;
            }
        }

        if (indices.IsComplete()) break;
    }

    try {
        uint32 gIdx = indices.Graphics.value();
        uint32 pIdx = indices.Present.value();
        uint32 cIdx = indices.Compute.value();

        // CZ_LOG(LogVulkanUtils, Info, "Indices valid: G{}, P{}, C{}", gIdx, pIdx, cIdx);
    } catch (const std::exception& e) {
        CZ_LOG(LogVulkanUtils, Error, "Crash during index access: {}", e.what());
    }

    return indices;
}

// Swapchain
SwapchainSupportDetails QuerySwapchainSupport(const vk::raii::PhysicalDevice& physicalDevice,
                                              const vk::raii::SurfaceKHR& surface) {
    try {
        if (!(*physicalDevice)) {
            CZ_LOG(LogVulkanUtils, Error, "QuerySwapchainSupport: PhysicalDevice handle is null!");
            return {};
        }
        if (!(*surface)) {
            CZ_LOG(LogVulkanUtils, Error, "QuerySwapchainSupport: Surface handle is null!");
            return {};
        }
        // Querying capabilities. If 'surface' is a "wild pointer" from a moved
        // Window object, this is where the 0xC0000409 occurs.
        return SwapchainSupportDetails{ physicalDevice.getSurfaceCapabilitiesKHR(*surface),
                                        physicalDevice.getSurfaceFormatsKHR(*surface),
                                        physicalDevice.getSurfacePresentModesKHR(*surface) };
    } catch (const std::exception& e) {
        CZ_LOG(LogVulkanUtils, Error, "Vulkan Error: %s", e.what());
        return {};
    }
}

vk::SurfaceFormatKHR
    ChooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats) {
    if (availableFormats.empty()) {
        CZ_LOG(LogVulkanUtils, Error, "No surface formats available!");
        return vk::Format::eUndefined;
    }

    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format == vk::Format::eB8G8R8A8Unorm &&
            availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
            return availableFormat;
        }
    }

    return availableFormats[0];
}

vk::PresentModeKHR ChooseSwapPresentMode(const EPresentMode inMode,
                                         const std::vector<vk::PresentModeKHR>& availableVKModes) {
    vk::PresentModeKHR targetVKMode = vk::PresentModeKHR::eFifo;

    switch (inMode) {
    case EPresentMode::Immediate:
        targetVKMode = vk::PresentModeKHR::eImmediate; // VSync OFF
        break;
    case EPresentMode::Mailbox:
        targetVKMode = vk::PresentModeKHR::eMailbox; // VSync OFF (Triple Buffering)
        break;
    case EPresentMode::FIFO:
        targetVKMode = vk::PresentModeKHR::eFifo; // VSync ON
        break;
    default:
        targetVKMode = vk::PresentModeKHR::eFifo;
        break;
    }

    for (const auto& available : availableVKModes) {
        if (available == targetVKMode) return targetVKMode;
    }

    return targetVKMode;
}

vk::Extent2D ChooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities, int pixelWidth,
                              int pixelHeight) {
    if (capabilities.currentExtent.width != (std::numeric_limits<uint32>::max)()) {
        return capabilities.currentExtent;
    }

    vk::Extent2D actualExtent = { static_cast<uint32>(pixelWidth),
                                  static_cast<uint32>(pixelHeight) };

    actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width,
                                    capabilities.maxImageExtent.width);
    actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height,
                                     capabilities.maxImageExtent.height);

    return actualExtent;
}

// Log
void LogPhysicalDeviceInfo(const vk::PhysicalDeviceProperties& properties) {

    CZ_LOG(LogVulkanUtils, Info, "Device Name: {}", properties.deviceName.data());
    CZ_LOG(LogVulkanUtils, Info, "Device Type: {}", vk::to_string(properties.deviceType));
    CZ_LOG(LogVulkanUtils, Info, "API Version: {}.{}.{}",
           VK_API_VERSION_MAJOR(properties.apiVersion), VK_API_VERSION_MINOR(properties.apiVersion),
           VK_API_VERSION_PATCH(properties.apiVersion));
}

void LogMemoryBudget(vk::raii::PhysicalDevice& physicalDevice) {
    vk::PhysicalDeviceMemoryProperties memProperties = physicalDevice.getMemoryProperties();

    CZ_LOG(LogVulkanUtils, Info, "--- GPU Memory Budget Report ---");

    for (uint32 i = 0; i < memProperties.memoryHeapCount; i++) {
        const auto& heap = memProperties.memoryHeaps[i];
        float sizeGB = static_cast<float>(heap.size) / (1024.0f * 1024.0f * 1024.0f);

        bool isDeviceLocal = (heap.flags & vk::MemoryHeapFlagBits::eDeviceLocal) ==
                             vk::MemoryHeapFlagBits::eDeviceLocal;

        std::string heapType;
        if (isDeviceLocal) {
            heapType = "Dedicated VRAM (Hardware) ";
        } else {
            heapType = "Shared System Memory (RAM)";
        }

        CZ_LOG(LogVulkanUtils, Info, "Heap {}: [{}] Size: {:.2f} GB", i, heapType, sizeGB);
    }

    CZ_LOG(LogVulkanUtils, Info, "---------------------------------");
}

vk::Format ToVKFormat(EPixelFormat format) {
    switch (format) {
    case EPixelFormat::RGBA8_UNORM:
        return vk::Format::eR8G8B8A8Unorm;
    case EPixelFormat::RGBA8_SRGB:
        return vk::Format::eR8G8B8A8Srgb;
    case EPixelFormat::BGRA8_UNORM:
        return vk::Format::eB8G8R8A8Unorm;
    case EPixelFormat::BGRA8_SRGB:
        return vk::Format::eB8G8R8A8Srgb;
    case EPixelFormat::D32_SFLOAT:
        return vk::Format::eD32Sfloat;
    case EPixelFormat::D24_UNORM_S8_UINT:
        return vk::Format::eD24UnormS8Uint;
    case EPixelFormat::D16_UNORM:
        return vk::Format::eD16Unorm;
    default:
        return vk::Format::eUndefined;
    }
}

// [Note] Reverse mapping might be useful for Swapchain -> RHI conversion
EPixelFormat FromVKFormat(vk::Format format) {
    switch (format) {
    case vk::Format::eR8G8B8A8Unorm:
        return EPixelFormat::RGBA8_UNORM;
    case vk::Format::eR8G8B8A8Srgb:
        return EPixelFormat::RGBA8_SRGB;
    case vk::Format::eB8G8R8A8Unorm:
        return EPixelFormat::BGRA8_UNORM;
    case vk::Format::eB8G8R8A8Srgb:
        return EPixelFormat::BGRA8_SRGB;
    case vk::Format::eD32Sfloat:
        return EPixelFormat::D32_SFLOAT;
    case vk::Format::eD24UnormS8Uint:
        return EPixelFormat::D24_UNORM_S8_UINT;
    case vk::Format::eD16Unorm:
        return EPixelFormat::D16_UNORM;
    default:
        return EPixelFormat::Unknown;
    }
}

bool IsDepthFormat(EPixelFormat format) {
    return format == EPixelFormat::D32_SFLOAT || format == EPixelFormat::D24_UNORM_S8_UINT ||
           format == EPixelFormat::D16_UNORM;
}

bool IsDepthFormat(vk::Format format) {
    return format == vk::Format::eD32Sfloat || format == vk::Format::eD24UnormS8Uint ||
           format == vk::Format::eD16Unorm;
}

} // namespace ChozoUtils::Vulkan

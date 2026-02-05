#pragma once

#include "Core.h"

#include <vulkan/vulkan_raii.hpp>

#include <optional>

struct FQueueFamilyIndices {
    std::optional<uint32_t> Graphics;
    std::optional<uint32_t> Present;
    std::optional<uint32_t> Compute;
    std::optional<uint32_t> Transfer;

    bool IsComplete() const {
        return Graphics.has_value() && Present.has_value();
    }
};

namespace ChozoUtils::Vulkan {

DECLARE_LOG_CATEGORY_EXTERN(LogVulkanUtils, Info);

namespace {
#ifdef NDEBUG
constexpr bool EnableValidationLayers = false;
#else
constexpr bool EnableValidationLayers = true;
#endif
} // namespace

inline const std::vector<const char*> ValidationLayers = {
    "VK_LAYER_KHRONOS_validation"};

template <typename T, typename Getter>
inline bool IsSupported(const std::vector<const char*>& required,
                        const std::vector<T>& available, Getter&& nameGetter) {
    return std::ranges::all_of(required, [&](const char* name) {
        return std::ranges::any_of(available, [&](const T& item) {
            return std::string_view(nameGetter(item)) == name;
        });
    });
}

inline bool CheckValidationLayerSupport(vk::raii::Context& context) {
    auto available = context.enumerateInstanceLayerProperties();
    return IsSupported(ValidationLayers, available,
                       [](auto& l) { return l.layerName; });
}

inline bool CheckInstanceExtensions(const vk::raii::Context& context,
                                    const std::vector<const char*>& required) {
    auto available = context.enumerateInstanceExtensionProperties();
    return IsSupported(required, available,
                       [](const auto& p) { return p.extensionName; });
}

inline FQueueFamilyIndices
    FindQueueFamilies(const vk::raii::PhysicalDevice& physicalDevice,
                      const vk::raii::SurfaceKHR& surface) {
    FQueueFamilyIndices indices;

    // find the index of the first queue family that supports graphics
    auto queueFamilyProps = physicalDevice.getQueueFamilyProperties();

    for (uint32_t i = 0; i < queueFamilyProps.size(); i++) {
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

        if (indices.IsComplete())
            break;
    }

    try {
        uint32_t gIdx = indices.Graphics.value();
        uint32_t pIdx = indices.Present.value();
        uint32_t cIdx = indices.Compute.value();

        CZ_LOG(LogVulkanUtils, Info, "Indices valid: G{}, P{}, C{}", gIdx, pIdx,
               cIdx);
    } catch (const std::exception& e) {
        CZ_LOG(LogVulkanUtils, Error, "Crash during index access: {}",
               e.what());
    }

    return indices;
}

// Swapchain
struct SwapchainSupportDetails {
    vk::SurfaceCapabilitiesKHR capabilities;
    std::vector<vk::SurfaceFormatKHR> formats;
    std::vector<vk::PresentModeKHR> presentModes;
};

inline SwapchainSupportDetails
    QuerySwapchainSupport(const vk::raii::PhysicalDevice& physicalDevice,
                          const vk::raii::SurfaceKHR& surface) {

    try {
        if (!(*physicalDevice)) {
            CZ_LOG(LogVulkanUtils, Error,
                   "QuerySwapchainSupport: PhysicalDevice handle is null!");
            return {};
        }
        if (!(*surface)) {
            CZ_LOG(LogVulkanUtils, Error,
                   "QuerySwapchainSupport: Surface handle is null!");
            return {};
        }
        // Querying capabilities. If 'surface' is a "wild pointer" from a moved
        // Window object, this is where the 0xC0000409 occurs.
        return SwapchainSupportDetails{
            physicalDevice.getSurfaceCapabilitiesKHR(*surface),
            physicalDevice.getSurfaceFormatsKHR(*surface),
            physicalDevice.getSurfacePresentModesKHR(*surface)};
    } catch (const std::exception& e) {
        CZ_LOG(LogVulkanUtils, Error, "Vulkan RAII Error: %s", e.what());
        return {};
    }
}

inline vk::SurfaceFormatKHR ChooseSwapSurfaceFormat(
    const std::vector<vk::SurfaceFormatKHR>& availableFormats) {
    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format == vk::Format::eB8G8R8A8Srgb &&
            availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
            return availableFormat;
        }
    }

    return availableFormats[0];
}

inline vk::PresentModeKHR ChooseSwapPresentMode(
    const std::vector<vk::PresentModeKHR>& availablePresentModes) {
    for (const auto& availablePresentMode : availablePresentModes) {
        if (availablePresentMode == vk::PresentModeKHR::eMailbox) {
            return availablePresentMode;
        }
    }
    return vk::PresentModeKHR::eFifo;
}

vk::Extent2D ChooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities,
                              int pixelWidth, int pixelHeight) {
    if (capabilities.currentExtent.width !=
        (std::numeric_limits<uint32_t>::max)()) {
        return capabilities.currentExtent;
    }

    vk::Extent2D actualExtent = {static_cast<uint32_t>(pixelWidth),
                                 static_cast<uint32_t>(pixelHeight)};

    actualExtent.width =
        std::clamp(actualExtent.width, capabilities.minImageExtent.width,
                   capabilities.maxImageExtent.width);
    actualExtent.height =
        std::clamp(actualExtent.height, capabilities.minImageExtent.height,
                   capabilities.maxImageExtent.height);

    return actualExtent;
}

// Log
inline void
    LogPhysicalDeviceInfo(const vk::PhysicalDeviceProperties& properties) {

    CZ_LOG(LogVulkanUtils, Info, "Device Name: {}",
           properties.deviceName.data());
    CZ_LOG(LogVulkanUtils, Info, "Device Type: {}",
           vk::to_string(properties.deviceType));
    CZ_LOG(LogVulkanUtils, Info, "API Version: {}.{}.{}",
           VK_API_VERSION_MAJOR(properties.apiVersion),
           VK_API_VERSION_MINOR(properties.apiVersion),
           VK_API_VERSION_PATCH(properties.apiVersion));
}

inline void LogMemoryBudget(vk::raii::PhysicalDevice& physicalDevice) {
    vk::PhysicalDeviceMemoryProperties memProperties =
        physicalDevice.getMemoryProperties();

    CZ_LOG(LogVulkanUtils, Info, "--- GPU Memory Budget Report ---");

    for (uint32_t i = 0; i < memProperties.memoryHeapCount; i++) {
        const auto& heap = memProperties.memoryHeaps[i];
        float sizeGB =
            static_cast<float>(heap.size) / (1024.0f * 1024.0f * 1024.0f);

        bool isDeviceLocal =
            (heap.flags & vk::MemoryHeapFlagBits::eDeviceLocal) ==
            vk::MemoryHeapFlagBits::eDeviceLocal;

        std::string heapType;
        if (isDeviceLocal) {
            heapType = "Dedicated VRAM (Hardware) ";
        } else {
            heapType = "Shared System Memory (RAM)";
        }

        CZ_LOG(LogVulkanUtils, Info, "Heap {}: [{}] Size: {:.2f} GB", i,
               heapType, sizeGB);
    }

    CZ_LOG(LogVulkanUtils, Info, "---------------------------------");
}
} // namespace ChozoUtils::Vulkan

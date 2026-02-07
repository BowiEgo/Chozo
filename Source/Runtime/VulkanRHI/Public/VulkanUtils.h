#pragma once

#include "CoreMinimal.h"
#include "RHITypes.h"

#include <optional>

DECLARE_LOG_CATEGORY_EXTERN(LogVulkanUtils, Info);

struct FQueueFamilyIndices {
    std::optional<uint32> Graphics;
    std::optional<uint32> Present;
    std::optional<uint32> Compute;
    std::optional<uint32> Transfer;

    bool IsComplete() const {
        return Graphics.has_value() && Present.has_value();
    }
};

namespace ChozoUtils::Vulkan {

vk::ShaderStageFlagBits StageToFlagBits(EShaderStage shaderStage);

namespace {
#ifdef NDEBUG
constexpr bool EnableValidationLayers = false;
#else
constexpr bool EnableValidationLayers = true;
#endif
} // namespace

const std::vector<const char*> ValidationLayers = {
    "VK_LAYER_KHRONOS_validation"};

template <typename T, typename Getter>
bool IsSupported(const std::vector<const char*>& required,
                 const std::vector<T>& available, Getter&& nameGetter);

bool CheckValidationLayerSupport(vk::raii::Context& context);

bool CheckInstanceExtensions(const vk::raii::Context& context,
                             const std::vector<const char*>& required);

FQueueFamilyIndices
    FindQueueFamilies(const vk::raii::PhysicalDevice& physicalDevice,
                      const vk::raii::SurfaceKHR& surface);

// Swapchain
struct SwapchainSupportDetails {
    vk::SurfaceCapabilitiesKHR capabilities;
    std::vector<vk::SurfaceFormatKHR> formats;
    std::vector<vk::PresentModeKHR> presentModes;
};

SwapchainSupportDetails
    QuerySwapchainSupport(const vk::raii::PhysicalDevice& physicalDevice,
                          const vk::raii::SurfaceKHR& surface);

vk::SurfaceFormatKHR ChooseSwapSurfaceFormat(
    const std::vector<vk::SurfaceFormatKHR>& availableFormats);

vk::PresentModeKHR ChooseSwapPresentMode(
    const std::vector<vk::PresentModeKHR>& availablePresentModes);

vk::Extent2D ChooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities,
                              int pixelWidth, int pixelHeight);

// Log
void LogPhysicalDeviceInfo(const vk::PhysicalDeviceProperties& properties);

void LogMemoryBudget(vk::raii::PhysicalDevice& physicalDevice);

vk::Format ToVulkanFormat(EPixelFormat format);

EPixelFormat FromVulkanFormat(vk::Format format);

} // namespace ChozoUtils::Vulkan

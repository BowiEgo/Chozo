#pragma once

#include "CoreMinimal.h"
#include "RHITypes.h"
#include "Ref.h"

#include <optional>

DECLARE_LOG_CATEGORY_EXTERN(LogVulkanUtils, Info);

struct FQueueFamilyIndices {
    std::optional<uint32> Graphics;
    std::optional<uint32> Present;
    std::optional<uint32> Compute;
    std::optional<uint32> Transfer;

    bool IsComplete() const { return Graphics.has_value() && Present.has_value(); }
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

const std::vector<const char*> ValidationLayers = { "VK_LAYER_KHRONOS_validation" };

template <typename T, typename Getter>
bool IsSupported(const std::vector<const char*>& required, const std::vector<T>& available,
                 Getter&& nameGetter);

bool CheckValidationLayerSupport(vk::raii::Context& context);

bool CheckInstanceExtensions(const vk::raii::Context& context,
                             const std::vector<const char*>& required);

FQueueFamilyIndices FindQueueFamilies(const vk::raii::PhysicalDevice& physicalDevice,
                                      const vk::raii::SurfaceKHR& surface);

// Swapchain
struct SwapchainSupportDetails {
    vk::SurfaceCapabilitiesKHR capabilities;
    std::vector<vk::SurfaceFormatKHR> formats;
    std::vector<vk::PresentModeKHR> presentModes;
};

SwapchainSupportDetails QuerySwapchainSupport(const vk::raii::PhysicalDevice& physicalDevice,
                                              const vk::raii::SurfaceKHR& surface);

vk::SurfaceFormatKHR
    ChooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats);

vk::PresentModeKHR ChooseSwapPresentMode(const EPresentMode inMode,
                                         const std::vector<vk::PresentModeKHR>& availableVKModes);

vk::Extent2D ChooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities, int pixelWidth,
                              int pixelHeight);

// Command
vk::CommandPoolCreateFlags MapCommandPoolFlags(ECommandPoolFlags rhiFlags);

// Image
void TransitionTextureLayout(const vk::CommandBuffer vkCmdBuffer, const vk::Image vkImage,
                             vk::ImageLayout oldLayout, vk::ImageLayout newLayout);

void SetupBarrierSync(vk::ImageMemoryBarrier2& barrier, vk::ImageLayout oldLayout,
                      vk::ImageLayout newLayout);

// Log
void LogPhysicalDeviceInfo(const vk::PhysicalDeviceProperties& properties);

void LogMemoryBudget(vk::raii::PhysicalDevice& physicalDevice);

vk::Format ToVKFormat(EPixelFormat format);

EPixelFormat FromVKFormat(vk::Format format);

bool IsDepthFormat(EPixelFormat format);

bool IsDepthFormat(vk::Format format);

// Shader
vk::Format ShaderDataTypeToVkFormat(EShaderDataType type);

// Pipeline
vk::PolygonMode GetVulkanPolygonMode(EPolygonMode mode);

} // namespace ChozoUtils::Vulkan

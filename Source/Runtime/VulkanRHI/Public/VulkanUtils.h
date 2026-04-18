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

// Samper
vk::Filter ToVKFilter(EFilter filter);

vk::SamplerAddressMode ToVKAddressMode(EAddressMode mode);

vk::SamplerMipmapMode ToVKMipmapMode(EMipmapMode mode);

// Image
vk::ImageLayout ToVkImageLayout(EImageLayout layout);

void TransitionImageLayout(const vk::CommandBuffer vkCmdBuffer, const vk::Image vkImage,
                           vk::ImageLayout oldLayout, vk::ImageLayout newLayout,
                           uint32_t baseArrayLayer = 0, uint32_t layerCount = 1);

void SetupBarrierSync(vk::ImageMemoryBarrier2& barrier, vk::ImageLayout oldLayout,
                      vk::ImageLayout newLayout);

vk::ImageViewType ToVkViewType(EImageViewType type);

vk::ImageAspectFlags GetImageAspectFlags(vk::Format format);

// Log
void LogPhysicalDeviceInfo(const vk::PhysicalDeviceProperties& properties);

void LogMemoryBudget(vk::raii::PhysicalDevice& physicalDevice);

vk::Format ToVkFormat(EPixelFormat format);

EPixelFormat FromVKFormat(vk::Format format);

bool IsDepthFormat(vk::Format format);

// Shader
vk::DescriptorType ToVkDescType(EUniformType type);

vk::Format ShaderDataTypeToVkFormat(EShaderDataFormat type);

// Pipeline
vk::PolygonMode GetVulkanPolygonMode(EPolygonMode mode);

vk::CullModeFlags ToVkCullMode(ECullMode mode);

vk::CompareOp ToVkCompareOp(ECompareOp op);

} // namespace ChozoUtils::Vulkan

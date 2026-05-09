#pragma once

#include <Runtime/RHI/RHITypes.h>

#include <vulkan/vulkan.h>

#include <optional>

namespace CZ {

struct QueueFamilyIndices {
    std::optional<uint32_t> Graphics;
    std::optional<uint32_t> Present;
    std::optional<uint32_t> Compute;
    std::optional<uint32_t> Transfer;

    bool IsComplete() const { return Graphics.has_value() && Present.has_value(); }
};
} // namespace CZ

namespace CZ::VulkanUtils {

VkShaderStageFlagBits StageToFlagBits(ShaderStage shaderStage);

const std::vector<const char*> ValidationLayers = { "VK_LAYER_KHRONOS_validation" };

// ---------- helper to check required extensions/layers ----------
template <typename T, typename NameGetter>
static bool IsSupported(const std::vector<const char*>& required, const std::vector<T>& available,
                        NameGetter&& nameGetter) {
    for (const auto* req : required) {
        bool found = false;
        for (const auto& item : available) {
            if (std::strcmp(nameGetter(item), req) == 0) {
                found = true;
                break;
            }
        }
        if (!found) return false;
    }
    return true;
}

bool CheckValidationLayerSupport();
bool CheckInstanceExtensions(const std::vector<const char*>& required);

QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);

const char* VkResultToString(VkResult result);

// Swapchain
struct SwapchainSupportDetails {
    VkSurfaceCapabilitiesKHR Capabilities;
    std::vector<VkSurfaceFormatKHR> Formats;
    std::vector<VkPresentModeKHR> PresentModes;
};

SwapchainSupportDetails QuerySwapchainSupport(VkPhysicalDevice physicalDevice,
                                              VkSurfaceKHR surface);

VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

VkPresentModeKHR ChooseSwapPresentMode(PresentMode inMode,
                                       const std::vector<VkPresentModeKHR>& availableVKModes);

VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, int pixelWidth,
                            int pixelHeight);

// Command
VkCommandPoolCreateFlags MapCommandPoolFlags(CommandPoolFlags rhiFlags);

// Sampler
VkFilter ToVKFilter(Filter filter);
VkSamplerAddressMode ToVKAddressMode(AddressMode mode);
VkSamplerMipmapMode ToVKMipmapMode(MipmapMode mode);

// Image
VkImageLayout ToVkImageLayout(ImageLayout layout);

void TransitionImageLayout(VkCommandBuffer vkCmdBuffer, VkImage vkImage, VkImageLayout oldLayout,
                           VkImageLayout newLayout, uint32_t baseArrayLayer = 0);

void SetupBarrierSync(VkImageMemoryBarrier2* barrier, VkImageLayout oldLayout,
                      VkImageLayout newLayout);

VkImageViewType ToVkViewType(ImageViewType type);
VkImageAspectFlags GetImageAspectFlags(VkFormat format);

// Log
void LogPhysicalDeviceInfo(const VkPhysicalDevice physicalDevice);
void LogMemoryBudget(const VkPhysicalDevice physicalDevice);

// Format
VkFormat ToVkFormat(PixelFormat format);
PixelFormat FromVKFormat(VkFormat format);
bool IsDepthFormat(VkFormat format);

// Shader
VkDescriptorType ToVkDescType(UniformType type);
VkFormat ShaderDataTypeToVkFormat(ShaderDataType type);

// Pipeline
VkPolygonMode GetVulkanPolygonMode(PolygonMode mode);
VkCullModeFlags ToVkCullMode(CullMode mode);
VkCompareOp ToVkCompareOp(CompareOp op);

} // namespace CZ::VulkanUtils
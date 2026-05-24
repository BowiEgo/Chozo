#include "VulkanUtils.hpp"

#include <Core/Header/Assert.hpp>
#include <Core/Log/LogMacros.hpp>

namespace CZ::VulkanUtils {

// Used for layer properties
static const char* layerNameGetter(const VkLayerProperties& layer) { return layer.layerName; }
// Used for extension properties
static const char* extensionNameGetter(const VkExtensionProperties& ext) {
    return ext.extensionName;
}

bool CheckValidationLayerSupport() {
    uint32_t count = 0;
    vkEnumerateInstanceLayerProperties(&count, nullptr);
    std::vector<VkLayerProperties> available(count);
    vkEnumerateInstanceLayerProperties(&count, available.data());
    return IsSupported(ValidationLayers, available, layerNameGetter);
}

bool CheckInstanceExtensions(const std::vector<const char*>& required) {
    uint32_t count = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);
    std::vector<VkExtensionProperties> available(count);
    vkEnumerateInstanceExtensionProperties(nullptr, &count, available.data());
    return IsSupported(required, available, extensionNameGetter);
}

// ---------- Queue Families ----------
QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) {
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilyProps(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount,
                                             queueFamilyProps.data());

    for (uint32_t i = 0; i < queueFamilyCount; i++) {
        const auto& qfp = queueFamilyProps[i];

        if (qfp.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.Graphics = i;
        }

        VkBool32 presentSupport = VK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupport);
        if (presentSupport) {
            indices.Present = i;
        }

        if (qfp.queueFlags & VK_QUEUE_COMPUTE_BIT) {
            if (!indices.Compute.has_value() || !(qfp.queueFlags & VK_QUEUE_GRAPHICS_BIT)) {
                indices.Compute = i;
            }
        }

        if (indices.IsComplete()) break;
    }

    if (indices.IsComplete()) {
        CZ_BACKEND_LOG(Info, "Queue families: G={}, P={}, C={}", indices.Graphics.value(),
                       indices.Present.value(), indices.Compute.value());
    } else {
        CZ_BACKEND_LOG(Error,
                       "Failed to find complete queue families (Graphics/Present/Compute missing)");
    }
    return indices;
}

const char* VkResultToString(VkResult result) {
    switch (result) {
        case VK_SUCCESS: return "VK_SUCCESS";
        case VK_NOT_READY: return "VK_NOT_READY";
        case VK_TIMEOUT: return "VK_TIMEOUT";
        case VK_EVENT_SET: return "VK_EVENT_SET";
        case VK_EVENT_RESET: return "VK_EVENT_RESET";
        case VK_INCOMPLETE: return "VK_INCOMPLETE";
        case VK_ERROR_OUT_OF_HOST_MEMORY: return "VK_ERROR_OUT_OF_HOST_MEMORY";
        case VK_ERROR_OUT_OF_DEVICE_MEMORY: return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
        case VK_ERROR_INITIALIZATION_FAILED: return "VK_ERROR_INITIALIZATION_FAILED";
        case VK_ERROR_DEVICE_LOST: return "VK_ERROR_DEVICE_LOST";
        case VK_ERROR_MEMORY_MAP_FAILED: return "VK_ERROR_MEMORY_MAP_FAILED";
        case VK_ERROR_LAYER_NOT_PRESENT: return "VK_ERROR_LAYER_NOT_PRESENT";
        case VK_ERROR_EXTENSION_NOT_PRESENT: return "VK_ERROR_EXTENSION_NOT_PRESENT";
        case VK_ERROR_FEATURE_NOT_PRESENT: return "VK_ERROR_FEATURE_NOT_PRESENT";
        case VK_ERROR_INCOMPATIBLE_DRIVER: return "VK_ERROR_INCOMPATIBLE_DRIVER";
        case VK_ERROR_TOO_MANY_OBJECTS: return "VK_ERROR_TOO_MANY_OBJECTS";
        case VK_ERROR_FORMAT_NOT_SUPPORTED: return "VK_ERROR_FORMAT_NOT_SUPPORTED";
        case VK_ERROR_FRAGMENTED_POOL: return "VK_ERROR_FRAGMENTED_POOL";
        case VK_ERROR_UNKNOWN: return "VK_ERROR_UNKNOWN";
        default: return "UNKNOWN_VK_RESULT";
    }
}

// ---------- Swapchain ----------
SwapchainSupportDetails QuerySwapchainSupport(VkPhysicalDevice physicalDevice,
                                              VkSurfaceKHR surface) {
    SwapchainSupportDetails details{};

    if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &details.Capabilities) !=
        VK_SUCCESS) {
        CZ_BACKEND_LOG(Error, "Failed to get surface capabilities");
        return {};
    }

    uint32_t formatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);
    if (formatCount > 0) {
        details.Formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount,
                                             details.Formats.data());
    }

    uint32_t presentModeCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr);
    if (presentModeCount > 0) {
        details.PresentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount,
                                                  details.PresentModes.data());
    }

    return details;
}

VkSurfaceFormatKHR
    ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
    if (availableFormats.empty()) {
        CZ_BACKEND_LOG(Error, "No surface formats available!");
        return { VK_FORMAT_UNDEFINED, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
    }

    for (const auto& format : availableFormats) {
        if (format.format == VK_FORMAT_B8G8R8A8_UNORM &&
            format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return format;
        }
    }
    return availableFormats[0];
}

VkPresentModeKHR ChooseSwapPresentMode(PresentMode inMode,
                                       const std::vector<VkPresentModeKHR>& availableModes) {
    VkPresentModeKHR target = VK_PRESENT_MODE_FIFO_KHR;
    switch (inMode) {
        case PresentMode::Immediate: target = VK_PRESENT_MODE_IMMEDIATE_KHR; break;
        case PresentMode::Mailbox: target = VK_PRESENT_MODE_MAILBOX_KHR; break;
        case PresentMode::FIFO: target = VK_PRESENT_MODE_FIFO_KHR; break;
        default: target = VK_PRESENT_MODE_FIFO_KHR; break;
    }

    for (const auto& mode : availableModes) {
        if (mode == target) return target;
    }
    return target;
}

VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, int pixelWidth,
                            int pixelHeight) {
    if (capabilities.currentExtent.width != UINT32_MAX) {
        return capabilities.currentExtent;
    }

    VkExtent2D actual = { static_cast<uint32_t>(pixelWidth), static_cast<uint32_t>(pixelHeight) };
    actual.width      = std::clamp(actual.width, capabilities.minImageExtent.width,
                                   capabilities.maxImageExtent.width);
    actual.height     = std::clamp(actual.height, capabilities.minImageExtent.height,
                                   capabilities.maxImageExtent.height);
    return actual;
}

// ---------- Command Pool ----------
VkCommandPoolCreateFlags MapCommandPoolFlags(CommandPoolFlags rhiFlags) {
    VkCommandPoolCreateFlags flags = 0;
    if (HasFlag(rhiFlags, CommandPoolFlags::Transient))
        flags |= VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    if (HasFlag(rhiFlags, CommandPoolFlags::ResetCommandBuffer))
        flags |= VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    if (HasFlag(rhiFlags, CommandPoolFlags::Protected))
        flags |= VK_COMMAND_POOL_CREATE_PROTECTED_BIT;
    return flags;
}

// ---------- Sampler ----------
VkFilter ToVKFilter(Filter filter) {
    switch (filter) {
        case Filter::Nearest: return VK_FILTER_NEAREST;
        case Filter::Linear: return VK_FILTER_LINEAR;
        default: return VK_FILTER_NEAREST;
    }
}

VkSamplerAddressMode ToVKAddressMode(AddressMode mode) {
    switch (mode) {
        case AddressMode::Repeat: return VK_SAMPLER_ADDRESS_MODE_REPEAT;
        case AddressMode::MirroredRepeat: return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
        case AddressMode::ClampToEdge: return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        case AddressMode::ClampToBorder: return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        default: return VK_SAMPLER_ADDRESS_MODE_REPEAT;
    }
}

VkSamplerMipmapMode ToVKMipmapMode(MipmapMode mode) {
    switch (mode) {
        case MipmapMode::Nearest: return VK_SAMPLER_MIPMAP_MODE_NEAREST;
        case MipmapMode::Linear: return VK_SAMPLER_MIPMAP_MODE_LINEAR;
        default: return VK_SAMPLER_MIPMAP_MODE_NEAREST;
    }
}

// ---------- Image ----------
VkImageLayout ToVkImageLayout(ImageLayout layout) {
    switch (layout) {
        case ImageLayout::Undefined: return VK_IMAGE_LAYOUT_UNDEFINED;
        case ImageLayout::General: return VK_IMAGE_LAYOUT_GENERAL;
        case ImageLayout::ColorAttachmentOptimal: return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        case ImageLayout::DepthStencilAttachmentOptimal:
            return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        case ImageLayout::DepthStencilReadOnlyOptimal:
            return VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
        case ImageLayout::ShaderReadOnlyOptimal: return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        case ImageLayout::TransferSrcOptimal: return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        case ImageLayout::TransferDstOptimal: return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        case ImageLayout::PresentSrc: return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        default: CZ_CORE_ASSERT(false, "Unknown ImageLayout"); return VK_IMAGE_LAYOUT_UNDEFINED;
    }
}

void TransitionImageLayout(VkCommandBuffer cmd, VkImage image, VkImageLayout oldLayout,
                           VkImageLayout newLayout, uint32_t baseArrayLayer) {
    if (oldLayout == newLayout) return;

    VkImageMemoryBarrier2 barrier{};
    barrier.sType                           = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
    barrier.oldLayout                       = oldLayout;
    barrier.newLayout                       = newLayout;
    barrier.srcQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
    barrier.image                           = image;
    barrier.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel   = 0;
    barrier.subresourceRange.levelCount     = VK_REMAINING_MIP_LEVELS;
    barrier.subresourceRange.baseArrayLayer = baseArrayLayer;
    barrier.subresourceRange.layerCount     = VK_REMAINING_ARRAY_LAYERS;

    SetupBarrierSync(&barrier, oldLayout, newLayout);

    VkDependencyInfo depInfo{};
    depInfo.sType                   = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    depInfo.imageMemoryBarrierCount = 1;
    depInfo.pImageMemoryBarriers    = &barrier;

    vkCmdPipelineBarrier2(cmd, &depInfo);
}

void SetupBarrierSync(VkImageMemoryBarrier2* barrier, VkImageLayout oldLayout,
                      VkImageLayout newLayout) {
    // default: all commands, write → write+read
    barrier->srcStageMask  = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
    barrier->srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT;
    barrier->dstStageMask  = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
    barrier->dstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED) {
        barrier->srcStageMask  = VK_PIPELINE_STAGE_2_NONE;
        barrier->srcAccessMask = VK_ACCESS_2_NONE;
    } else if (oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
        barrier->srcStageMask  = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
        barrier->srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier->srcStageMask  = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
        barrier->srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) {
        barrier->srcStageMask  = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
        barrier->srcAccessMask = VK_ACCESS_2_TRANSFER_READ_BIT;
    }

    if (newLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR) {
        barrier->dstStageMask  = VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT;
        barrier->dstAccessMask = VK_ACCESS_2_NONE;
    } else if (newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier->dstStageMask  = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
        barrier->dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
    } else if (newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier->dstStageMask  = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
        barrier->dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
    } else if (newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) {
        barrier->dstStageMask  = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
        barrier->dstAccessMask = VK_ACCESS_2_TRANSFER_READ_BIT;
    } else if (newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
        barrier->dstStageMask  = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
        barrier->dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
    } else if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
        barrier->dstStageMask  = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT |
                                 VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT;
        barrier->dstAccessMask = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    } else {
        CZ_BACKEND_LOG(Warning, "General transition from {} to {} using AllCommands fallback.",
                       static_cast<uint32_t>(oldLayout), static_cast<uint32_t>(newLayout));
    }
}

VkImageViewType ToVkViewType(ImageViewType type) {
    switch (type) {
        case ImageViewType::View1D: return VK_IMAGE_VIEW_TYPE_1D;
        case ImageViewType::View2D: return VK_IMAGE_VIEW_TYPE_2D;
        case ImageViewType::View3D: return VK_IMAGE_VIEW_TYPE_3D;
        case ImageViewType::ViewCube: return VK_IMAGE_VIEW_TYPE_CUBE;
        case ImageViewType::View1DArray: return VK_IMAGE_VIEW_TYPE_1D_ARRAY;
        case ImageViewType::View2DArray: return VK_IMAGE_VIEW_TYPE_2D_ARRAY;
        case ImageViewType::ViewCubeArray: return VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
        default: CZ_CORE_ASSERT(false, "Unknown ImageViewType"); return VK_IMAGE_VIEW_TYPE_2D;
    }
}

VkImageAspectFlags GetImageAspectFlags(VkFormat format) {
    switch (format) {
        case VK_FORMAT_D16_UNORM:
        case VK_FORMAT_D32_SFLOAT: return VK_IMAGE_ASPECT_DEPTH_BIT;
        case VK_FORMAT_D16_UNORM_S8_UINT:
        case VK_FORMAT_D24_UNORM_S8_UINT:
        case VK_FORMAT_D32_SFLOAT_S8_UINT:
            return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
        case VK_FORMAT_S8_UINT: return VK_IMAGE_ASPECT_STENCIL_BIT;
        default: return VK_IMAGE_ASPECT_COLOR_BIT;
    }
}

// ---------- Format conversion ----------
VkFormat ToVkFormat(PixelFormat format) {
    switch (format) {
        case PixelFormat::R8_UNORM: return VK_FORMAT_R8_UNORM;
        case PixelFormat::R16_UNORM: return VK_FORMAT_R16_UNORM;
        case PixelFormat::R16F: return VK_FORMAT_R16_SFLOAT;
        case PixelFormat::R32F: return VK_FORMAT_R32_SFLOAT;
        case PixelFormat::RG8_UNORM: return VK_FORMAT_R8G8_UNORM;
        case PixelFormat::RG16_UNORM: return VK_FORMAT_R16G16_UNORM;
        case PixelFormat::RG16F: return VK_FORMAT_R16G16_SFLOAT;
        case PixelFormat::RG32F: return VK_FORMAT_R32G32_SFLOAT;
        case PixelFormat::RGBA8_UNORM: return VK_FORMAT_R8G8B8A8_UNORM;
        case PixelFormat::RGBA8_SRGB: return VK_FORMAT_R8G8B8A8_SRGB;
        case PixelFormat::BGRA8_UNORM: return VK_FORMAT_B8G8R8A8_UNORM;
        case PixelFormat::BGRA8_SRGB: return VK_FORMAT_B8G8R8A8_SRGB;
        case PixelFormat::RGBA16_UNORM: return VK_FORMAT_R16G16B16A16_UNORM;
        case PixelFormat::RGBA16F: return VK_FORMAT_R16G16B16A16_SFLOAT;
        case PixelFormat::RGBA32F: return VK_FORMAT_R32G32B32A32_SFLOAT;
        case PixelFormat::RGB9E5: return VK_FORMAT_E5B9G9R9_UFLOAT_PACK32;
        case PixelFormat::R11G11B10F: return VK_FORMAT_B10G11R11_UFLOAT_PACK32;
        case PixelFormat::D16_UNORM: return VK_FORMAT_D16_UNORM;
        case PixelFormat::D24_UNORM_S8_UINT: return VK_FORMAT_D24_UNORM_S8_UINT;
        case PixelFormat::D32_SFLOAT: return VK_FORMAT_D32_SFLOAT;
        default: return VK_FORMAT_UNDEFINED;
    }
}

PixelFormat FromVKFormat(VkFormat format) {
    switch (format) {
        case VK_FORMAT_R8_UNORM: return PixelFormat::R8_UNORM;
        case VK_FORMAT_R16_UNORM: return PixelFormat::R16_UNORM;
        case VK_FORMAT_R16_SFLOAT: return PixelFormat::R16F;
        case VK_FORMAT_R32_SFLOAT: return PixelFormat::R32F;
        case VK_FORMAT_R8G8_UNORM: return PixelFormat::RG8_UNORM;
        case VK_FORMAT_R16G16_UNORM: return PixelFormat::RG16_UNORM;
        case VK_FORMAT_R16G16_SFLOAT: return PixelFormat::RG16F;
        case VK_FORMAT_R32G32_SFLOAT: return PixelFormat::RG32F;
        case VK_FORMAT_R8G8B8A8_UNORM: return PixelFormat::RGBA8_UNORM;
        case VK_FORMAT_R8G8B8A8_SRGB: return PixelFormat::RGBA8_SRGB;
        case VK_FORMAT_B8G8R8A8_UNORM: return PixelFormat::BGRA8_UNORM;
        case VK_FORMAT_B8G8R8A8_SRGB: return PixelFormat::BGRA8_SRGB;
        case VK_FORMAT_R16G16B16A16_UNORM: return PixelFormat::RGBA16_UNORM;
        case VK_FORMAT_R16G16B16A16_SFLOAT: return PixelFormat::RGBA16F;
        case VK_FORMAT_R32G32B32A32_SFLOAT: return PixelFormat::RGBA32F;
        case VK_FORMAT_E5B9G9R9_UFLOAT_PACK32: return PixelFormat::RGB9E5;
        case VK_FORMAT_B10G11R11_UFLOAT_PACK32: return PixelFormat::R11G11B10F;
        case VK_FORMAT_D16_UNORM: return PixelFormat::D16_UNORM;
        case VK_FORMAT_D24_UNORM_S8_UINT: return PixelFormat::D24_UNORM_S8_UINT;
        case VK_FORMAT_D32_SFLOAT: return PixelFormat::D32_SFLOAT;
        default: return PixelFormat::Unknown;
    }
}

bool IsDepthFormat(VkFormat format) {
    return format == VK_FORMAT_D32_SFLOAT || format == VK_FORMAT_D24_UNORM_S8_UINT ||
           format == VK_FORMAT_D16_UNORM;
}

// ---------- Shader ----------
VkDescriptorType ToVkDescType(UniformType type) {
    switch (type) {
        case UniformType::Sampler: return VK_DESCRIPTOR_TYPE_SAMPLER;
        case UniformType::Image: return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        case UniformType::CombinedImageSampler: return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        case UniformType::UniformBuffer: return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        case UniformType::StorageImage: return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        case UniformType::StorageBuffer: return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        case UniformType::InputAttachment: return VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
        default:
            CZ_BACKEND_LOG(Error, "Unsupported uniform type");
            return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    }
}

VkFormat ShaderDataTypeToVkFormat(ShaderDataType type) {
    switch (type) {
        case ShaderDataType::Float: return VK_FORMAT_R32_SFLOAT;
        case ShaderDataType::Float2: return VK_FORMAT_R32G32_SFLOAT;
        case ShaderDataType::Float3: return VK_FORMAT_R32G32B32_SFLOAT;
        case ShaderDataType::Float4: return VK_FORMAT_R32G32B32A32_SFLOAT;
        case ShaderDataType::Int: return VK_FORMAT_R32_SINT;
        case ShaderDataType::Int2: return VK_FORMAT_R32G32_SINT;
        case ShaderDataType::Int3: return VK_FORMAT_R32G32B32_SINT;
        case ShaderDataType::Int4: return VK_FORMAT_R32G32B32A32_SINT;
        case ShaderDataType::Bool: return VK_FORMAT_R32_SINT;
        default: CZ_BACKEND_LOG(Error, "Unknown shader data type"); return VK_FORMAT_UNDEFINED;
    }
}

// ---------- Pipeline ----------
VkShaderStageFlagBits StageToFlagBits(ShaderStage stage) {
    switch (stage) {
#define GENERATE_CASE(ENUM, LOWER, UPPER, SHORT, GLSL, VULKAN, VULKAN_UPPER)                       \
    case ShaderStage::ENUM: return VK_SHADER_STAGE_##VULKAN_UPPER##_BIT;
        FOREACH_SHADER_STAGE(GENERATE_CASE)
#undef GENERATE_CASE
        default: return static_cast<VkShaderStageFlagBits>(0);
    }
}

VkPolygonMode GetVulkanPolygonMode(PolygonMode mode) {
    switch (mode) {
        case PolygonMode::Fill: return VK_POLYGON_MODE_FILL;
        case PolygonMode::Line: return VK_POLYGON_MODE_LINE;
        default: return VK_POLYGON_MODE_FILL;
    }
}

VkCullModeFlags ToVkCullMode(CullMode mode) {
    switch (mode) {
        case CullMode::None: return VK_CULL_MODE_NONE;
        case CullMode::Front: return VK_CULL_MODE_FRONT_BIT;
        case CullMode::Back: return VK_CULL_MODE_BACK_BIT;
        case CullMode::FrontAndBack: return VK_CULL_MODE_FRONT_AND_BACK;
        default: return VK_CULL_MODE_BACK_BIT;
    }
}

VkCompareOp ToVkCompareOp(CompareOp op) {
    switch (op) {
        case CompareOp::Never: return VK_COMPARE_OP_NEVER;
        case CompareOp::Less: return VK_COMPARE_OP_LESS;
        case CompareOp::Equal: return VK_COMPARE_OP_EQUAL;
        case CompareOp::LessOrEqual: return VK_COMPARE_OP_LESS_OR_EQUAL;
        case CompareOp::Greater: return VK_COMPARE_OP_GREATER;
        case CompareOp::NotEqual: return VK_COMPARE_OP_NOT_EQUAL;
        case CompareOp::GreaterOrEqual: return VK_COMPARE_OP_GREATER_OR_EQUAL;
        case CompareOp::Always: return VK_COMPARE_OP_ALWAYS;
        default: return VK_COMPARE_OP_ALWAYS;
    }
}

// ---------- Logging ----------
void LogPhysicalDeviceInfo(const VkPhysicalDevice physicalDevice) {
    VkPhysicalDeviceProperties props;
    vkGetPhysicalDeviceProperties(physicalDevice, &props);

    CZ_BACKEND_LOG(Info, "Device Name: {}", props.deviceName);
    CZ_BACKEND_LOG(Info, "API Version: {}.{}.{}", VK_API_VERSION_MAJOR(props.apiVersion),
                   VK_API_VERSION_MINOR(props.apiVersion), VK_API_VERSION_PATCH(props.apiVersion));
}

void LogMemoryBudget(const VkPhysicalDevice physicalDevice) {
    VkPhysicalDeviceMemoryProperties memProps;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProps);

    CZ_BACKEND_LOG(Info, "--- GPU Memory Budget Report ---");
    for (uint32_t i = 0; i < memProps.memoryHeapCount; i++) {
        float sizeGB =
            static_cast<float>(memProps.memoryHeaps[i].size) / (1024.0f * 1024.0f * 1024.0f);
        bool isLocal = (memProps.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) != 0;
        CZ_BACKEND_LOG(Info, "Heap {}: [{}] Size: {:.2f} GB", i,
                       isLocal ? "Dedicated VRAM" : "Shared System Memory", sizeGB);
    }
    CZ_BACKEND_LOG(Info, "---------------------------------");
}

} // namespace CZ::VulkanUtils
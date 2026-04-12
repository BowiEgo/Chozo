#include "VulkanUtils.h"

DEFINE_LOG_CATEGORY(LogVulkanUtils);

namespace ChozoUtils::Vulkan {

vk::ShaderStageFlagBits StageToFlagBits(EShaderStage shaderStage) {
    switch (shaderStage) {
#define GENERATE_CASE(ENUM, LOWER, UPPER, SHORT, GLSL, VULKAN)                                     \
    case EShaderStage::ENUM: return vk::ShaderStageFlagBits::e##VULKAN;
        FOREACH_SHADER_STAGE(GENERATE_CASE)
#undef GENERATE_CASE
        default: return static_cast<vk::ShaderStageFlagBits>(0);
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
        default: targetVKMode = vk::PresentModeKHR::eFifo; break;
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

    actualExtent.width  = std::clamp(actualExtent.width, capabilities.minImageExtent.width,
                                     capabilities.maxImageExtent.width);
    actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height,
                                     capabilities.maxImageExtent.height);

    return actualExtent;
}

// Command
vk::CommandPoolCreateFlags MapCommandPoolFlags(ECommandPoolFlags rhiFlags) {
    vk::CommandPoolCreateFlags vkFlags;

    if (HasFlag(rhiFlags, ECommandPoolFlags::Transient))
        vkFlags |= vk::CommandPoolCreateFlagBits::eTransient;

    if (HasFlag(rhiFlags, ECommandPoolFlags::ResetCommandBuffer))
        vkFlags |= vk::CommandPoolCreateFlagBits::eResetCommandBuffer;

    if (HasFlag(rhiFlags, ECommandPoolFlags::Protected))
        vkFlags |= vk::CommandPoolCreateFlagBits::eProtected;

    return vkFlags;
}

// Samper
vk::Filter ToVKFilter(EFilter filter) {
    switch (filter) {
        case EFilter::Nearest: return vk::Filter::eNearest;
        case EFilter::Linear: return vk::Filter::eLinear;
        default: return vk::Filter::eNearest;
    }
}

vk::SamplerAddressMode ToVKAddressMode(EAddressMode mode) {
    switch (mode) {
        case EAddressMode::Repeat: return vk::SamplerAddressMode::eRepeat;
        case EAddressMode::MirroredRepeat: return vk::SamplerAddressMode::eMirroredRepeat;
        case EAddressMode::ClampToEdge: return vk::SamplerAddressMode::eClampToEdge;
        case EAddressMode::ClampToBorder: return vk::SamplerAddressMode::eClampToBorder;
        default: return vk::SamplerAddressMode::eRepeat;
    }
}

vk::SamplerMipmapMode ToVKMipmapMode(EMipmapMode mode) {
    switch (mode) {
        case EMipmapMode::Nearest: return vk::SamplerMipmapMode::eNearest;
        case EMipmapMode::Linear: return vk::SamplerMipmapMode::eLinear;
        default: return vk::SamplerMipmapMode::eNearest;
    }
}

// Image
vk::ImageLayout ToVkImageLayout(EImageLayout layout) {
    switch (layout) {
        case EImageLayout::Undefined: return vk::ImageLayout::eUndefined;
        case EImageLayout::General: return vk::ImageLayout::eGeneral;
        case EImageLayout::ColorAttachmentOptimal: return vk::ImageLayout::eColorAttachmentOptimal;
        case EImageLayout::DepthStencilAttachmentOptimal:
            return vk::ImageLayout::eDepthStencilAttachmentOptimal;
        case EImageLayout::DepthStencilReadOnlyOptimal:
            return vk::ImageLayout::eDepthStencilReadOnlyOptimal;
        case EImageLayout::ShaderReadOnlyOptimal: return vk::ImageLayout::eShaderReadOnlyOptimal;
        case EImageLayout::TransferSrcOptimal: return vk::ImageLayout::eTransferSrcOptimal;
        case EImageLayout::TransferDstOptimal: return vk::ImageLayout::eTransferDstOptimal;
        case EImageLayout::PresentSrc: return vk::ImageLayout::ePresentSrcKHR;

        case EImageLayout::Unknown:
        default:
            CZ_CORE_ASSERT(false, "Unknown or unsupported EImageLayout!");
            return vk::ImageLayout::eUndefined;
    }
}

void TransitionImageLayout(const vk::CommandBuffer vkCmdBuffer, const vk::Image vkImage,
                           vk::ImageLayout oldLayout, vk::ImageLayout newLayout) {
    if (oldLayout == newLayout) return;

    vk::ImageMemoryBarrier2 barrier;
    barrier.setOldLayout(oldLayout)
        .setNewLayout(newLayout)
        .setSrcQueueFamilyIndex(vk::QueueFamilyIgnored)
        .setDstQueueFamilyIndex(vk::QueueFamilyIgnored)
        .setImage(vkImage)
        .setSubresourceRange(
            vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));

    // Automatically deduce stages and access masks based on layouts
    SetupBarrierSync(barrier, oldLayout, newLayout);

    vk::DependencyInfo depInfo;
    depInfo.setImageMemoryBarriers(barrier);

    vkCmdBuffer.pipelineBarrier2(depInfo);
}

// This function automatically sets srcAccessMask, dstAccessMask, srcStageMask, and dstStageMask
// based on the old and new layouts. For simplicity, we handle common cases here.
void SetupBarrierSync(vk::ImageMemoryBarrier2& barrier, vk::ImageLayout oldLayout,
                      vk::ImageLayout newLayout) {
    // Default to All Commands if no specific match is found (Safe but slow).
    barrier.setSrcStageMask(vk::PipelineStageFlagBits2::eAllCommands)
        .setSrcAccessMask(vk::AccessFlagBits2::eMemoryWrite)
        .setDstStageMask(vk::PipelineStageFlagBits2::eAllCommands)
        .setDstAccessMask(vk::AccessFlagBits2::eMemoryWrite | vk::AccessFlagBits2::eMemoryRead);

    // --- Source Layout Transitions (Old Layout) ---

    // Initial state or freshly allocated memory
    if (oldLayout == vk::ImageLayout::eUndefined) {
        barrier.setSrcStageMask(vk::PipelineStageFlagBits2::eNone)
            .setSrcAccessMask(vk::AccessFlagBits2::eNone);
    }
    // From RenderTarget (Color Attachment)
    else if (oldLayout == vk::ImageLayout::eColorAttachmentOptimal) {
        barrier.setSrcStageMask(vk::PipelineStageFlagBits2::eColorAttachmentOutput)
            .setSrcAccessMask(vk::AccessFlagBits2::eColorAttachmentWrite);
    }
    // Previous operation was a transfer (copy) write
    else if (oldLayout == vk::ImageLayout::eTransferDstOptimal) {
        barrier.setSrcStageMask(vk::PipelineStageFlagBits2::eTransfer)
            .setSrcAccessMask(vk::AccessFlagBits2::eTransferWrite);
    }
    // Previous operation was a transfer (copy) read
    else if (oldLayout == vk::ImageLayout::eTransferSrcOptimal) {
        barrier.setSrcStageMask(vk::PipelineStageFlagBits2::eTransfer)
            .setSrcAccessMask(vk::AccessFlagBits2::eTransferRead);
    }

    // --- Destination Layout Transitions (New Layout) ---

    // To Present Source (The 1000001002 case)
    if (newLayout == vk::ImageLayout::ePresentSrcKHR) {
        barrier.setDstStageMask(vk::PipelineStageFlagBits2::eBottomOfPipe)
            .setDstAccessMask(vk::AccessFlagBits2::eNone);
    }
    // To Shader Read Only (Combined Image Sampler)
    else if (newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
        barrier.setDstStageMask(vk::PipelineStageFlagBits2::eFragmentShader)
            .setDstAccessMask(vk::AccessFlagBits2::eShaderRead);
    }
    // Target for data upload (vkCmdCopyBufferToImage)
    else if (newLayout == vk::ImageLayout::eTransferDstOptimal) {
        barrier.setDstStageMask(vk::PipelineStageFlagBits2::eTransfer)
            .setDstAccessMask(vk::AccessFlagBits2::eTransferWrite);
    }
    // Source for data download or blitting
    else if (newLayout == vk::ImageLayout::eTransferSrcOptimal) {
        barrier.setDstStageMask(vk::PipelineStageFlagBits2::eTransfer)
            .setDstAccessMask(vk::AccessFlagBits2::eTransferRead);
    }
    // To RenderTarget (Color Attachment)
    else if (newLayout == vk::ImageLayout::eColorAttachmentOptimal) {
        barrier.setDstStageMask(vk::PipelineStageFlagBits2::eColorAttachmentOutput)
            .setDstAccessMask(vk::AccessFlagBits2::eColorAttachmentWrite);
    }
    // Target for depth/stencil testing
    else if (newLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal) {
        barrier
            .setDstStageMask(vk::PipelineStageFlagBits2::eEarlyFragmentTests |
                             vk::PipelineStageFlagBits2::eLateFragmentTests)
            .setDstAccessMask(vk::AccessFlagBits2::eDepthStencilAttachmentWrite);
    }
    // Fallback for unhandled transitions using default masks
    else {
        CZ_LOG(LogVulkanUtils, Warning,
               "General transition from {0} to {1} using AllCommands fallback.", (uint32)oldLayout,
               (uint32)newLayout);
    }
}

vk::ImageViewType ToVkViewType(EImageViewType type) {
    switch (type) {
        case EImageViewType::View1D: return vk::ImageViewType::e1D;
        case EImageViewType::View2D: return vk::ImageViewType::e2D;
        case EImageViewType::View3D: return vk::ImageViewType::e3D;
        case EImageViewType::ViewCube: return vk::ImageViewType::eCube;
        case EImageViewType::View1DArray: return vk::ImageViewType::e1DArray;
        case EImageViewType::View2DArray: return vk::ImageViewType::e2DArray;
        case EImageViewType::ViewCubeArray: return vk::ImageViewType::eCubeArray;
        default: CZ_CORE_ASSERT(false, "Unknown EImageViewType"); return vk::ImageViewType::e2D;
    }
}

vk::ImageAspectFlags GetImageAspectFlags(vk::Format format) {
    switch (format) {
        // --- Depth & Stencil Formats ---
        case vk::Format::eD16Unorm:
        case vk::Format::eD32Sfloat: return vk::ImageAspectFlagBits::eDepth;

        case vk::Format::eD16UnormS8Uint:
        case vk::Format::eD24UnormS8Uint:
        case vk::Format::eD32SfloatS8Uint:
            return vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil;

        case vk::Format::eS8Uint: return vk::ImageAspectFlagBits::eStencil;

        // --- Everything else is Color ---
        default: return vk::ImageAspectFlagBits::eColor;
    }
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
        float sizeGB     = static_cast<float>(heap.size) / (1024.0f * 1024.0f * 1024.0f);

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

vk::Format ToVkFormat(EPixelFormat format) {
    switch (format) {
        // Single-channel
        case EPixelFormat::R8_UNORM: return vk::Format::eR8Unorm;
        case EPixelFormat::R16_UNORM: return vk::Format::eR16Unorm;
        case EPixelFormat::R16F: return vk::Format::eR16Sfloat;
        case EPixelFormat::R32F: return vk::Format::eR32Sfloat;

        // Dual-channel
        case EPixelFormat::RG8_UNORM: return vk::Format::eR8G8Unorm;
        case EPixelFormat::RG16_UNORM: return vk::Format::eR16G16Unorm;
        case EPixelFormat::RG16F: return vk::Format::eR16G16Sfloat;
        case EPixelFormat::RG32F: return vk::Format::eR32G32Sfloat;

        // 8-bit 4-channel
        case EPixelFormat::RGBA8_UNORM: return vk::Format::eR8G8B8A8Unorm;
        case EPixelFormat::RGBA8_SRGB: return vk::Format::eR8G8B8A8Srgb;
        case EPixelFormat::BGRA8_UNORM: return vk::Format::eB8G8R8A8Unorm;
        case EPixelFormat::BGRA8_SRGB: return vk::Format::eB8G8R8A8Srgb;

        // 16-bit 4-channel
        case EPixelFormat::RGBA16_UNORM: return vk::Format::eR16G16B16A16Unorm;
        case EPixelFormat::RGBA16F: return vk::Format::eR16G16B16A16Sfloat;

        // 32-bit 4-channel
        case EPixelFormat::RGBA32F: return vk::Format::eR32G32B32A32Sfloat;

        // Special packed RGB
        case EPixelFormat::RGB9E5: return vk::Format::eE5B9G9R9UfloatPack32;
        case EPixelFormat::R11G11B10F: return vk::Format::eB10G11R11UfloatPack32;

        // Depth/stencil
        case EPixelFormat::D16_UNORM: return vk::Format::eD16Unorm;
        case EPixelFormat::D24_UNORM_S8_UINT: return vk::Format::eD24UnormS8Uint;
        case EPixelFormat::D32_SFLOAT: return vk::Format::eD32Sfloat;

        default: return vk::Format::eUndefined;
    }
}

EPixelFormat FromVKFormat(vk::Format format) {
    switch (format) {
        // Single-channel
        case vk::Format::eR8Unorm: return EPixelFormat::R8_UNORM;
        case vk::Format::eR16Unorm: return EPixelFormat::R16_UNORM;
        case vk::Format::eR16Sfloat: return EPixelFormat::R16F;
        case vk::Format::eR32Sfloat: return EPixelFormat::R32F;

        // Dual-channel
        case vk::Format::eR8G8Unorm: return EPixelFormat::RG8_UNORM;
        case vk::Format::eR16G16Unorm: return EPixelFormat::RG16_UNORM;
        case vk::Format::eR16G16Sfloat: return EPixelFormat::RG16F;
        case vk::Format::eR32G32Sfloat: return EPixelFormat::RG32F;

        // 8-bit 4-channel
        case vk::Format::eR8G8B8A8Unorm: return EPixelFormat::RGBA8_UNORM;
        case vk::Format::eR8G8B8A8Srgb: return EPixelFormat::RGBA8_SRGB;
        case vk::Format::eB8G8R8A8Unorm: return EPixelFormat::BGRA8_UNORM;
        case vk::Format::eB8G8R8A8Srgb: return EPixelFormat::BGRA8_SRGB;

        // 16-bit 4-channel
        case vk::Format::eR16G16B16A16Unorm: return EPixelFormat::RGBA16_UNORM;
        case vk::Format::eR16G16B16A16Sfloat: return EPixelFormat::RGBA16F;

        // 32-bit 4-channel
        case vk::Format::eR32G32B32A32Sfloat: return EPixelFormat::RGBA32F;

        // Special packed RGB (note: Vulkan stores these as BGR order, but we map to our RGB-named
        // enums)
        case vk::Format::eE5B9G9R9UfloatPack32: return EPixelFormat::RGB9E5;
        case vk::Format::eB10G11R11UfloatPack32: return EPixelFormat::R11G11B10F;

        // Depth/stencil
        case vk::Format::eD16Unorm: return EPixelFormat::D16_UNORM;
        case vk::Format::eD24UnormS8Uint: return EPixelFormat::D24_UNORM_S8_UINT;
        case vk::Format::eD32Sfloat: return EPixelFormat::D32_SFLOAT;

        default: return EPixelFormat::Unknown;
    }
}

bool IsDepthFormat(vk::Format format) {
    return format == vk::Format::eD32Sfloat || format == vk::Format::eD24UnormS8Uint ||
           format == vk::Format::eD16Unorm;
}

// Shader
vk::DescriptorType ToVkDescType(EUniformType type) {
    switch (type) {
        case EUniformType::Sampler: return vk::DescriptorType::eSampler;
        case EUniformType::Image: return vk::DescriptorType::eSampledImage;
        case EUniformType::CombinedImageSampler: return vk::DescriptorType::eCombinedImageSampler;
        case EUniformType::UniformBuffer: return vk::DescriptorType::eUniformBuffer;
        case EUniformType::StorageImage: return vk::DescriptorType::eStorageImage;
        case EUniformType::StorageBuffer: return vk::DescriptorType::eStorageBuffer;
        case EUniformType::InputAttachment: return vk::DescriptorType::eInputAttachment;
        case EUniformType::PushConstant:
        default:
            CZ_LOG(LogVulkanUtils, Error, "Unsupported uniform type");
            return vk::DescriptorType::eUniformBuffer;
    }
}

vk::Format ShaderDataTypeToVkFormat(EShaderDataFormat type) {
    switch (type) {
        case EShaderDataFormat::Float: return vk::Format::eR32Sfloat;
        case EShaderDataFormat::Float2: return vk::Format::eR32G32Sfloat;
        case EShaderDataFormat::Float3: return vk::Format::eR32G32B32Sfloat;
        case EShaderDataFormat::Float4: return vk::Format::eR32G32B32A32Sfloat;
        case EShaderDataFormat::Int: return vk::Format::eR32Sint;
        case EShaderDataFormat::Int2: return vk::Format::eR32G32Sint;
        case EShaderDataFormat::Int3: return vk::Format::eR32G32B32Sint;
        case EShaderDataFormat::Int4: return vk::Format::eR32G32B32A32Sint;
        case EShaderDataFormat::Bool: return vk::Format::eR32Sint;
        default:
            CZ_LOG(LogVulkanUtils, Error, "Unsupported shader data type");
            return vk::Format::eUndefined;
    }
}

// Pipeline
vk::PolygonMode GetVulkanPolygonMode(EPolygonMode mode) {
    switch (mode) {
        case EPolygonMode::Fill: return vk::PolygonMode::eFill;
        case EPolygonMode::Line: return vk::PolygonMode::eLine;
        case EPolygonMode::Point: return vk::PolygonMode::ePoint;
        default: return vk::PolygonMode::eFill;
    }
}

} // namespace ChozoUtils::Vulkan

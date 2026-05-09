#pragma once

#include <Runtime/RHI/Texture.h>

#include <Core/Memory/Memory.h>
#include <Core/Platform/Platform.h>

#include "VulkanUtils.h"

#ifdef CZ_PLATFORM_MACOS
    #define VK_USE_PLATFORM_METAL_EXT
#endif
#include <vulkan/vulkan.h>

namespace CZ {

class VulkanTextureObj : public TextureObj {
public:
    VulkanTextureObj(const Device device, const TextureSpecification& spec);
    VulkanTextureObj(const Device device, const TextureSpecification& spec, Image image);
    ~VulkanTextureObj() override;
};

} // namespace CZ
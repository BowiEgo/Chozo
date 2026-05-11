#pragma once

#include <Runtime/RHI/Texture.h>

namespace CZ {

class VulkanTextureObj : public TextureObj {
public:
    VulkanTextureObj(const Device device, const TextureSpecification& spec);
    VulkanTextureObj(const Device device, const TextureSpecification& spec, Image image);
    ~VulkanTextureObj() override;
};

} // namespace CZ
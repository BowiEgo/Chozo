#pragma once

#include <Runtime/RHI/RHIAPI.h>

#include "../Source/Runtime/RHI/RHIAPIObj.h"

namespace CZ {

class VulkanAPIObj : public RHIAPIObj {
public:
    VulkanAPIObj(GraphicsContext ctx) : RHIAPIObj(ctx) {}
    ~VulkanAPIObj() override {}

    void BeginRendering(CommandList cmdList, std::vector<Texture>& targets, bool bClear,
                        uint32_t faceIndex) override;

    void DrawFrame(CommandList cmdList, RecordCallback recordCallback) override;

    void EndRendering(CommandList cmdList) override;

    void TransitionImageLayout(CommandList cmdList, Image image, const ImageLayout newLayout,
                               uint32_t baseArrayLayer) override;
};

} // namespace CZ
#pragma once

#include <Runtime/RHI/RHIAPI.h>

namespace CZ {

class RHIAPIObj {
public:
    RHIAPIObj(GraphicsContext ctx) : m_GraphicsContext(ctx) {}
    virtual ~RHIAPIObj() = default;

    virtual void BeginRendering(CommandList cmdList, std::vector<Texture>& targets, bool bClear,
                                uint32_t faceIndex) = 0;

    virtual void DrawFrame(CommandList cmdList, RecordCallback recordCallback) = 0;

    virtual void EndRendering(CommandList cmdList) = 0;

    virtual void TransitionImageLayout(CommandList cmdList, Image image,
                                       const ImageLayout newLayout, uint32_t baseArrayLayer) = 0;

    GraphicsContext GetGraphicsContext() const { return m_GraphicsContext; }

protected:
    GraphicsContext m_GraphicsContext;
};

} // namespace CZ
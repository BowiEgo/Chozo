#pragma once
#include <vector>

#include <cstdint>

#include <Runtime/RHI/RHIAPI.hpp>

namespace CZ {

class VulkanAPIObj : public RHIAPIObj {
public:
    VulkanAPIObj(GraphicsContext ctx) : RHIAPIObj(ctx) {}
    ~VulkanAPIObj() override;

    void BeginRendering(CommandList cmdList, std::vector<Texture>& targets, bool bClear,
                        uint32_t faceIndex, Texture depthTarget) override;

    void BeginGPUFrame(CommandList cmdList) override;
    void BeginGPUTiming(CommandList cmdList) override;
    void EndGPUTiming(CommandList cmdList) override;
    GPUTiming GetGPUTiming() const override { return m_GpuTiming; }

    void DrawFrame(CommandList cmdList, RecordCallback recordCallback) override;

private:
    // GPU frame timing: one timestamp pair per in-flight frame, reset from the command buffer
    // (hostQueryReset is not enabled on this device) and read back with the availability flag.
    static constexpr uint32_t kFramesInFlight = 3;
    VkQueryPool m_TimestampPool               = VK_NULL_HANDLE;
    uint32_t m_FrameIndex                     = 0;
    uint32_t m_PassIndex                      = 0;
    float m_TimestampPeriod                   = 0.0f;
    bool m_TimestampChecked                   = false;
    bool m_TimestampSupported                 = false;
    GPUTiming m_GpuTiming{};

public:
    void EndRendering(CommandList cmdList) override;

    void TransitionImageLayout(CommandList cmdList, Image image, const ImageLayout newLayout,
                               uint32_t baseArrayLayer) override;
};

} // namespace CZ
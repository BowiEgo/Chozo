#pragma once
#include <functional>
#include <string>

#include <cstdint>

#include "Runtime/RHI/GraphicsBuffer.hpp"
#include <Runtime/RHI/CommandList.hpp>
#include <Runtime/RHI/Device.hpp>
#include <Runtime/RHI/FrameBuffer.hpp>
#include <Runtime/RHI/GraphicsContext.hpp>
#include <Runtime/RHI/Pipeline.hpp>
#include <Runtime/RHI/Sampler.hpp>
#include <Runtime/RHI/ShaderRes.hpp>
#include <Runtime/RHI/Texture.hpp>
#include <vector>

namespace CZ {

using RecordCallback = std::function<void(uint32)>;

class RHIAPIObj {
public:
    RHIAPIObj(GraphicsContext ctx) : m_GraphicsContext(ctx) {}
    virtual ~RHIAPIObj() = default;

    /// GPU frame timing. Begin/End bracket the passes inside the recording callback (timestamps are
    /// only valid while the command buffer records); results arrive a few frames later and are read
    /// back without ever blocking. bValid stays false while unsupported, so callers can omit it.
    struct GPUTiming {
        float FrameSeconds = 0.0f;
        bool bValid        = false;
    };

    virtual void BeginGPUTiming(CommandList cmdList) = 0;
    virtual void EndGPUTiming(CommandList cmdList)   = 0;
    virtual GPUTiming GetGPUTiming() const           = 0;

    // depthTarget is optional: pass the framebuffer's depth attachment so that pipelines which
    // enable depth testing actually have one bound. A pipeline declaring depth test while the
    // render pass binds no depth attachment makes the GPU discard every fragment.
    virtual void BeginRendering(CommandList cmdList, std::vector<Texture>& targets, bool bClear,
                                uint32_t faceIndex = 0, Texture depthTarget = {}) = 0;

    virtual void DrawFrame(CommandList cmdList, RecordCallback recordCallback) = 0;

    virtual void EndRendering(CommandList cmdList) = 0;

    virtual void TransitionImageLayout(CommandList cmdList, Image image,
                                       const ImageLayout newLayout,
                                       uint32_t baseArrayLayer = 0) = 0;

    GraphicsContext GetGraphicsContext() const { return m_GraphicsContext; }

    void WaitIdle() { m_GraphicsContext->GetDevice()->WaitIdle(); }

    Sampler GetSampler(const SamplerSpecification spec) {
        return m_GraphicsContext->GetDevice()->GetOrCreateSampler(spec);
    }

    Scope<FrameBufferObj> CreateFrameBuffer(const FrameBufferSpecification& spec) {
        return m_GraphicsContext->GetDevice()->CreateFrameBuffer(spec);
    }

    Scope<ShaderResObj> CreateShaderRes(const ShaderResSpecification& spec,
                                        const std::vector<uint32_t>* binary) {
        return m_GraphicsContext->GetDevice()->CreateShaderRes(spec, binary);
    }

    Scope<PipelineObj> CreatePipeline(const PipelineSpecification& spec,
                                      const std::vector<ShaderRes>& shaders,
                                      const ShaderReflection& reflection) {
        return m_GraphicsContext->GetDevice()->CreatePipeline(spec, shaders, reflection);
    }

    Scope<GraphicsBufferObj> CreateGraphicsBuffer(const GraphicsBufferSpecification& spec,
                                                  const Buffer* initialData = nullptr) {
        return m_GraphicsContext->GetDevice()->CreateGraphicsBuffer(spec, initialData);
    }

protected:
    GraphicsContext m_GraphicsContext;
};

/// Owns the backend API object; every caller only sees `operator->`.
struct RHIAPI {
public:
    RHIAPI(const RHIAPI&)            = delete;
    RHIAPI& operator=(const RHIAPI&) = delete;

    static RHIAPI& Get();

    static bool Init(GraphicsContext ctx, std::string& err);

    static void Shutdown();

    RHIAPIObj* operator->() const { return m_Obj.get(); }

private:
    RHIAPI()  = default;
    ~RHIAPI() = default;

    Scope<RHIAPIObj> m_Obj;
};
} // namespace CZ

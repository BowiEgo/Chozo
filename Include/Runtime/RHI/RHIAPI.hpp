#pragma once

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

    virtual void BeginRendering(CommandList cmdList, std::vector<Texture>& targets, bool bClear,
                                uint32_t faceIndex = 0) = 0;

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

    FrameBuffer CreateFrameBuffer(const FrameBufferSpecification& spec) {
        return m_GraphicsContext->GetDevice()->CreateFrameBuffer(spec);
    }

    ShaderRes CreateShaderRes(const ShaderResSpecification& spec,
                              const std::vector<uint32_t>* binary) {
        return m_GraphicsContext->GetDevice()->CreateShaderRes(spec, binary);
    }

    Pipeline CreatePipeline(const PipelineSpecification& spec,
                            const std::vector<ShaderRes>& shaders,
                            const ShaderReflection& reflection) {
        return m_GraphicsContext->GetDevice()->CreatePipeline(spec, shaders, reflection);
    }

    GraphicsBuffer CreateGraphicsBuffer(const GraphicsBufferSpecification& spec,
                                        const Buffer* initialData = nullptr) {
        return m_GraphicsContext->GetDevice()->CreateGraphicsBuffer(spec, initialData);
    }

protected:
    GraphicsContext m_GraphicsContext;
};

struct RHIAPI : Handle<class RHIAPIObj> {
public:
    RHIAPI(const RHIAPI&)            = delete;
    RHIAPI& operator=(const RHIAPI&) = delete;

    static RHIAPI& Get();

    static bool Init(GraphicsContext ctx, std::string& err);

    static void Shutdown();

private:
    RHIAPI() = default;
    ~RHIAPI() {
        if (m_Obj) Shutdown();
    }
};
} // namespace CZ

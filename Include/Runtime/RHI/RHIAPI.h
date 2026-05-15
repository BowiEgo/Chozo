#pragma once

#include <Runtime/RHI/Device.h>
#include <Runtime/RHI/GraphicsContext.h>
#include <Runtime/RHI/Texture.h>

namespace CZ {

using RecordCallback = std::function<void(uint32)>;

class RHIAPIObj;

struct RHIAPI : Handle<class RHIAPIObj> {
public:
    static RHIAPI& Get();

    RHIAPI(const RHIAPI&)            = delete;
    RHIAPI& operator=(const RHIAPI&) = delete;

    bool Init(GraphicsContext ctx, std::string& err);

    void Shutdown();

    void WaitIdle() const;

    void BeginRendering(CommandList cmdList, std::vector<Texture>& targets, bool bClear,
                        uint32_t faceIndex = 0);

    void DrawFrame(CommandList cmdList, RecordCallback recordCallback);

    void EndRendering(CommandList cmdList);

    void TransitionImageLayout(CommandList cmdList, const Image image, const ImageLayout newLayout,
                               uint32_t baseArrayLayer = 0);

    GraphicsContext GetGraphicsContext() const;

    Sampler GetSampler(const SamplerSpecification spec);

private:
    RHIAPI() = default;
    ~RHIAPI() {
        if (m_Obj) Shutdown();
    }
};
} // namespace CZ

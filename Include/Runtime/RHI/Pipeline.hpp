#pragma once

#include <Core/Header/Handle.hpp>
#include <Runtime/RHI/RHITypes.hpp>
#include <Runtime/RHI/SetLayout.hpp>
#include <Runtime/RHI/ShaderRes.hpp>

namespace CZ {

struct PipelineSpecification {
    std::string Name;

    std::vector<PixelFormat> ColorFormats;
    PixelFormat DepthFormat  = PixelFormat::D32_SFLOAT;
    PolygonMode PolygonMode  = PolygonMode::Fill;
    CullMode CullMode        = CullMode::Back;
    bool bDepthTestEnable    = true;
    bool bDepthWriteEnable   = true;
    CompareOp DepthCompareOp = CompareOp::Less;

    std::unordered_map<uint32_t, SetLayout> SetLayouts;
    VertexBufferLayout VertexLayout;
    std::vector<PushConstantRange> PushConstantRanges;
};

class PipelineObj {
    friend class Handle<PipelineObj>;

public:
    PipelineObj(const PipelineSpecification& spec) : m_Spec(spec) {}

    virtual ~PipelineObj() {};

    PolygonMode GetPolygonMode() const { return m_Spec.PolygonMode; }

    const SetLayout GetSetLayout(uint32_t set) {
        if (set < m_SetLayouts.size()) return m_SetLayouts[set];
        return SetLayout();
    }

protected:
    PipelineSpecification m_Spec;
    std::vector<SetLayout> m_SetLayouts;
};

struct Pipeline : Handle<class PipelineObj> {
    template <typename T> T* As() { return static_cast<T*>(InternalHandleReader::Unwrap(*this)); }
};

} // namespace CZ

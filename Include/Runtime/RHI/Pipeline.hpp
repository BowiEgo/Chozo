#pragma once

#include <Core/Header/Handle.hpp>
#include <Runtime/RHI/RHITypes.hpp>
#include <Runtime/RHI/SetLayout.hpp>

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
};

class PipelineObj {
public:
    PipelineObj(const PipelineSpecification& spec) : m_Spec(spec) {}

    virtual ~PipelineObj() {};

protected:
    PipelineSpecification m_Spec;

    std::vector<SetLayout> m_SetLayouts;
};

struct Pipeline : Handle<class PipelineObj> {
    template <typename T> T* As() { return static_cast<T*>(InternalHandleReader::Unwrap(*this)); }
};

} // namespace CZ

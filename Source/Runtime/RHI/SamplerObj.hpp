#pragma once

#include <Runtime/RHI/Sampler.hpp>

namespace CZ {

class SamplerObj {
public:
    SamplerObj(const SamplerSpecification& spec) : m_Spec(spec) {}
    virtual ~SamplerObj() = default;

protected:
    SamplerSpecification m_Spec;
};

} // namespace CZ

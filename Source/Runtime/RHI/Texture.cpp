#include <Runtime/RHI/RHIAPI.hpp>
#include <Runtime/RHI/Texture.hpp>

namespace CZ {

Sampler TextureObj::GetSampler(const SamplerSpecification spec) {
    return RHIAPI::Get()->GetSampler(spec);
}

} // namespace CZ
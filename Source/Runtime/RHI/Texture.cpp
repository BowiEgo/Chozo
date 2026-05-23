#include <Runtime/RHI/RHIAPI.hpp>
#include <Runtime/RHI/Texture.hpp>

namespace CZ {

DEFINE_LOG_CATEGORY_STATIC(LogTexture, Info);

DEFINE_HANDLE_DESTROY(TextureObj)

Sampler TextureObj::GetSampler(const SamplerSpecification spec) {
    return RHIAPI::Get()->GetSampler(spec);
}

} // namespace CZ
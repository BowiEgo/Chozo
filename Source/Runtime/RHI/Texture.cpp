#include <Runtime/RHI/RHIAPI.hpp>
#include <Runtime/RHI/Texture.hpp>

namespace CZ {

template <> void Handle<TextureObj>::Destroy() {
    if (m_Obj) {
        m_Obj->m_Image.Destroy();

        Delete(m_Obj);
        m_Obj = nullptr;
    }
}

Sampler TextureObj::GetSampler(const SamplerSpecification spec) {
    return RHIAPI::Get()->GetSampler(spec);
}

} // namespace CZ
#include "Texture.h"

#include "RHIAPI.h"

DEFINE_LOG_CATEGORY(LogTexture);

CTexture::CTexture(const FTextureSpecification& spec) : m_Spec(spec) {
    // CZ_LOG(LogTexture, Trace, "Creating Texture {} ...", m_Spec.Name);
}

CTexture::CTexture(const FTextureSpecification& spec, const IRHITexture* texture) : m_Spec(spec) {
    m_Resource.reset(const_cast<IRHITexture*>(texture));

    // CZ_LOG(LogTexture, Trace, "Creating Texture {} ...", m_Spec.Name);
}

IRHITexture* CTexture::GetOrCreateResource() {
    if (m_Resource) {
        return m_Resource.get();
    }

    if (m_Spec.Type == ETextureType::Texture2D)
        m_Resource = TScope<IRHITexture2D>(IRHIAPI::CreateTexture2D(m_Spec).get());

    CZ_LOG(LogTexture, Info, "RHI Texture: {} created.", m_Spec.Name);

    return m_Resource.get();
}

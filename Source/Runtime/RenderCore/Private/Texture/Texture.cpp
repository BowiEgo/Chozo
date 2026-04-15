#include "Texture.h"

#include "RHIAPI.h"

DEFINE_LOG_CATEGORY(LogTexture);

CTexture::CTexture(const FTextureSpecification& spec) : m_Spec(spec) {
    // CZ_LOG(LogTexture, Trace, "Creating Texture {} ...", m_Spec.Name);
}

CTexture::CTexture(const FTextureSpecification& spec, const FBuffer& data)
    : m_Spec(spec), m_Data(data) {
    // CZ_LOG(LogTexture, Trace, "Creating Texture {} ...", m_Spec.Name);
}

IRHITexture* CTexture::GetOrCreateResource() {
    if (m_Resource) {
        return m_Resource.get();
    }

    if (m_Data.Data) {
        m_Resource = IRHIAPI::CreateTexture(m_Spec, m_Data);
    } else {
        m_Resource = IRHIAPI::CreateTexture(m_Spec);
    }

    CZ_LOG(LogTexture, Info, "RHI Texture: {} created.", m_Spec.Name);

    return m_Resource.get();
}

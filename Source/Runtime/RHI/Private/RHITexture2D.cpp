#include "RHITexture2D.h"

DEFINE_LOG_CATEGORY(LogRHITexture2D);

IRHITexture2D::IRHITexture2D(const FTextureSpecification& spec) : m_Spec(spec) {}

IRHITexture2D::~IRHITexture2D() {
    CZ_LOG(LogRHITexture2D, Trace, "RHITexture2D: {} destroying...", m_Spec.Name);
}

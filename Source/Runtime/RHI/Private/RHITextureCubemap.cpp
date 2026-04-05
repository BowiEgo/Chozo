#include "RHITextureCubemap.h"

DEFINE_LOG_CATEGORY(LogRHITextureCubemap);

IRHITextureCubemap::~IRHITextureCubemap() {
    // CZ_LOG(LogRHITextureCubemap, Trace, "RHITextureCubemap: {} destroying...", m_Spec.Name);
}

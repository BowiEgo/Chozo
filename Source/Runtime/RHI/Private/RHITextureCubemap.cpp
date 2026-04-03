#include "RHITextureCubemap.h"

DEFINE_LOG_CATEGORY(LogRHITextureCubemap);

IRHITextureCubemap::IRHITextureCubemap(const WeakRef<IRHIDevice> device,
                                       const FTextureCubemapSpecification& spec)
    : IRHIResource(device), m_Spec(spec) {}

IRHITextureCubemap::~IRHITextureCubemap() {
    // CZ_LOG(LogRHITextureCubemap, Trace, "RHITextureCubemap: {} destroying...", m_Spec.Name);
}

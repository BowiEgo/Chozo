#include "RHISampler.h"

DEFINE_LOG_CATEGORY(LogRHISampler);

IRHISampler::IRHISampler(const WeakRef<IRHIDevice> device, const FSamplerSpecification& spec)
    : IRHIResource(device), m_Spec(spec) {}

IRHISampler::~IRHISampler() {
    // CZ_LOG(LogRHISampler, Trace, "RHISampler: destroying...");
}
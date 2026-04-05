#include "RHIDescriptorSet.h"

DEFINE_LOG_CATEGORY(LogRHIDescriptorSet);

IRHIDescriptorSet::IRHIDescriptorSet(const WeakRef<IRHIDevice> device) : IRHIResource(device) {}

IRHIDescriptorSet::~IRHIDescriptorSet() {
    // CZ_LOG(LogRHIDescriptor, Trace, "RHIDescriptor: destroying...");
}
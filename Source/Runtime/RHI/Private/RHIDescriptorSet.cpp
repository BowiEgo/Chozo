#include "RHIDescriptorSet.h"

DEFINE_LOG_CATEGORY(LogRHIDescriptorSet);

IRHIDescriptorSet::IRHIDescriptorSet(const WeakRef<IRHIDevice> device,
                                     TRef<IRHISetLayout> setLayout,
                                     const std::vector<FDescriptorBinding>& bindings)
    : IRHIResource(device), m_SetLayout(setLayout), m_ResourceBindings(bindings) {}

IRHIDescriptorSet::~IRHIDescriptorSet() {
    // CZ_LOG(LogRHIDescriptor, Trace, "RHIDescriptor: destroying...");
}
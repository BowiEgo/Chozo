#include "RHISetLayout.h"

DEFINE_LOG_CATEGORY(LogRHISetLayout);

IRHISetLayout::IRHISetLayout(const WeakRef<IRHIDevice> device) : IRHIResource(device) {}

IRHISetLayout::~IRHISetLayout() {
    // CZ_LOG(LogRHISetLayout, Trace, "RHISetLayout: destroying...");
}
#include "RHICommandPool.h"

DEFINE_LOG_CATEGORY(LogRHICommandPool);

IRHICommandPool::IRHICommandPool(const FCommandPoolSpecification& spec) : m_Spec(spec) {}

IRHICommandPool::~IRHICommandPool() {
    CZ_LOG(LogRHICommandPool, Trace, "RHICommandPool destroying...");
}

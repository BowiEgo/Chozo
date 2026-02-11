#include "RHICommandPool.h"

DEFINE_LOG_CATEGORY(LogRHICommandPool);

IRHICommandPool::IRHICommandPool(const FRHICommandPoolCreateInfo& info) : m_Info(info) {}

IRHICommandPool::~IRHICommandPool() {
    CZ_LOG(LogRHICommandPool, Trace, "RHICommandPool destroying...");
}

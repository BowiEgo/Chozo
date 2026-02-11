#include "RHISwapchain.h"

DEFINE_LOG_CATEGORY(LogRHISwapchain);

IRHISwapchain::IRHISwapchain(const FRHISwapchainCreateInfo& info) : m_Info(info) {}

IRHISwapchain::~IRHISwapchain() { CZ_LOG(LogRHISwapchain, Trace, "RHISwapchain destroying..."); }

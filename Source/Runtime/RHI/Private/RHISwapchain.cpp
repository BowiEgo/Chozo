#include "RHISwapchain.h"

DEFINE_LOG_CATEGORY(LogRHISwapchain);

IRHISwapchain::IRHISwapchain(const FSwapchainSpecification& spec) : m_Spec(spec) {}

IRHISwapchain::~IRHISwapchain() { CZ_LOG(LogRHISwapchain, Trace, "RHISwapchain destroying..."); }

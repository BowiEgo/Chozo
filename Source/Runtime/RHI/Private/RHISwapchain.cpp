#include "RHISwapchain.h"

IRHISwapchain::IRHISwapchain(const FRHISwapchainCreateInfo& info)
    : m_Info(info) {}

IRHISwapchain::~IRHISwapchain() = default;
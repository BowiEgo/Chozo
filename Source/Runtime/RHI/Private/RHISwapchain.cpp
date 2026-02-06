#include "RHISwapchain.h"

IRHISwapchain::IRHISwapchain(const FRHISwapchainCreateInfo& info)
    : m_Data(info) {}

IRHISwapchain::~IRHISwapchain() = default;
#include "VulkanContext.h"

#include "VulkanAPI.h"
#include "VulkanUtils.h"

DEFINE_LOG_CATEGORY(LogVulkanContext);

extern "C" {
VULKAN_API IRHIContext* CreateVulkanContext(const FContextSpec& spec) {
    return new CVulkanContext(spec);
}
}

CVulkanContext::CVulkanContext(const FContextSpec& spec) : IRHIContext(spec) {
    CVulkanAPI::Init(m_Spec.WindowRequiredExtensions, m_Spec.NativeWindow);

    {
        FDeviceSpecification spec;
        spec.AppName = "ChozoEngine";
        spec.AppVersion = 1;
        m_Device = IRHIAPI::CreateDevice(this, spec);
    }

    {
        FSwapchainSpecification spec;
        spec.FrameBufferSize = m_Spec.FrameBufferSize;
        spec.NativeWindow = m_Spec.NativeWindow;
        m_Swapchain = IRHIAPI::CreateSwapchain(this, spec);
    }
}

CVulkanContext::~CVulkanContext() {}

#include "GraphicsContext.h"

#include "ModuleUtils.h"
#include "RHIDevice.h"
#include "RHISwapchain.h"

DEFINE_LOG_CATEGORY(LogGraphicsContext);

CGraphicsContext::CGraphicsContext(const FRHIWindowInfo& windowInfo) : m_WindowInfo(windowInfo) {
    Init();
}

CGraphicsContext::~CGraphicsContext() {
    CZ_LOG(LogGraphicsContext, Trace, "GraphicsContext destroying...");
}

void CGraphicsContext::Init() {
    std::string libName = GetPlatformLibName("VulkanRHI");
    if (m_RHIModule.Load(libName)) {
        FRHICreateInfo RHIInfo;
        RHIInfo.NativeWindow = m_WindowInfo.NativeWindow;
        RHIInfo.RequiredExtensions = m_WindowInfo.RequiredExtensions;

        m_RHI = TScope<IRHI>(
            m_RHIModule.Invoke<IRHI*(const FRHICreateInfo&)>("CreateVulkanRHI", RHIInfo));

        if (m_RHI) {
            CZ_LOG(LogGraphicsContext, Info, "RHI created.");

            FRHIDeviceCreateInfo deviceInfo;
            deviceInfo.AppName = "ChozoEngine";
            deviceInfo.AppVersion = 1;
            m_RHI->CreateDevice(deviceInfo);

            FRHISwapchainCreateInfo swapchainInfo;
            swapchainInfo.FrameBufferSize = m_WindowInfo.FrameBufferSize;
            swapchainInfo.NativeWindow = m_WindowInfo.NativeWindow;
            TRef<IRHISwapchain> swapchain = m_RHI->CreateSwapchain(swapchainInfo);
        }
    }
}

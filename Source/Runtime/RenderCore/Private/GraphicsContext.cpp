#include "GraphicsContext.h"

#include "Module.h"
#include "ModuleUtils.h"
#include "ShaderManager.h"

DEFINE_LOG_CATEGORY(LogGraphicsContext);

CGraphicsContext* CGraphicsContext::s_Instance = nullptr;

CGraphicsContext::CGraphicsContext(IRendererWindow* windowHandle)
    : m_Window(windowHandle) {

    CZ_CORE_ASSERT(!s_Instance, "Graphics Context already exists!");
    s_Instance = this;
}

CGraphicsContext::~CGraphicsContext() { s_Instance = nullptr; }

void CGraphicsContext::Init() {

    CModule vulkanModule;

    if (vulkanModule.Load("VulkanRHI.dll")) {
        FRHICreateInfo info;
        info.WindowHandle = m_Window->GetWindowWrapper();
        info.RequiredExtensions = m_Window->GetRequiredExtensions();

        vulkanModule.Load(GetPlatformLibName("VulkanRHI"));
        m_RHI = TScope<IRHI>(vulkanModule.Invoke<IRHI*(const FRHICreateInfo&)>(
            "CreateVulkanRHI", info));

        if (m_RHI) {
            CZ_LOG(LogGraphicsContext, Info, "RHI created.");
            CShaderManager::Init();
        }
    }
}

CGraphicsContext& CGraphicsContext::Get() {
    CZ_CORE_ASSERT(s_Instance, "Graphics Context is not initialized!");
    return *s_Instance;
}
#include "RenderEngine.h"

#include "Module.h"
#include "ModuleUtils.h"
#include "RHIDevice.h"
#include "RendererAPI.h"
#include "ShaderManager.h"

DEFINE_LOG_CATEGORY(LogRenderEngine);

void CRenderEngine::Init() {
    CZ_LOG(LogRenderEngine, Trace, "Render Engine Initializing...");

    if (m_Window) {
        // m_Context = CGraphicsContext::Create(m_Window);
        // m_Context->Init();

        CModule vulkanModule;
        if (vulkanModule.Load("VulkanRHI.dll")) {

            FRHIDeviceCreateInfo info;

            info.WindowHandle = m_Window->GetWindowWrapper();
            info.RequiredExtensions = m_Window->GetRequiredExtensions();

            vulkanModule.Load(GetPlatformLibName("VulkanRHI"));
            m_Device = TRef<IRHIDevice>(vulkanModule.Invoke<PFN_CreateDevice>(
                "CreateVulkanDevice", info));

            if (m_Device) {
                CZ_LOG(LogRenderEngine, Info,
                       "Successfully created RHI Device via CModule.");
            }
        }
    }

    CShaderManager::Init(m_Device);

    CZ_LOG(LogRenderEngine, Info, "Render Engine Initialized.");
}
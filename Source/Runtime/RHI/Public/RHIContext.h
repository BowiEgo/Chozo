#pragma once

#include "RHIDevice.h"
#include "RHIExport.h"
#include "RHISwapchain.h"
#include "Ref.h"

DECLARE_LOG_CATEGORY_EXTERN(LogRHIContext, Info);

struct FContextSpec {
    FExtent2D FrameBufferSize;
    void* NativeWindow;
    std::vector<const char*> WindowRequiredExtensions;
};

class RHI_API IRHIContext {
public:
    IRHIContext(const FContextSpec& spec);
    virtual ~IRHIContext();

    TRef<IRHIDevice> GetDevice() const { return m_Device; }
    TRef<IRHISwapchain> GetSwapchain() const { return m_Swapchain; }
    uint32 GetCurrentImageIndex() const { return m_ImageIndex; }
    TRef<IRHITexture2D> GetTarget() const { return m_Target; }

    void SetCurrentImageIndex(const uint32 index) { m_ImageIndex = index; }
    void SetTarget(const TRef<IRHITexture2D>& target) { m_Target = target; }

protected:
    FContextSpec m_Spec;

    TRef<IRHIDevice> m_Device;
    TRef<IRHISwapchain> m_Swapchain;
    // std::vector<TRef<CVulkanCommandList>> m_FrameCommandLists;

    uint32 m_ImageIndex;
    bool m_IsFirstRenderingInFrame = true;
    TRef<IRHITexture2D> m_Target;
};
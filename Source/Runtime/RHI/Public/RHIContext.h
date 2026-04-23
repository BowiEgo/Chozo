#pragma once

#include "RHIExport.h"
#include "RHISwapchain.h"
#include "Ref.h"

DECLARE_LOG_CATEGORY_EXTERN(LogRHIContext, Info);

class IRHIDevice;
class FRDGTexture;

struct FContextSpec {
    FExtent2D FrameBufferSize;
    void* NativeWindow;
    std::vector<const char*> WindowRequiredExtensions;
};

class RHI_API IRHIContext {
    static const int MAX_FRAMES_IN_FLIGHT = 3;

public:
    IRHIContext(const FContextSpec& spec);
    virtual ~IRHIContext();

    TRef<IRHIDevice> GetDevice() const { return m_Device; }
    TRef<IRHISwapchain> GetSwapchain() const { return m_Swapchain; }
    float GetCurrentFrame() const { return m_Frame; }
    uint32 GetCurrentFrameIndex() const { return m_FrameIndex; }
    uint32 GetCurrentImageIndex() const { return m_ImageIndex; }
    uint32 GetMaxFramesInFlight() const { return MAX_FRAMES_IN_FLIGHT; }
    IRHITexture* GetTarget() const { return m_Target; }
    const std::vector<IRHITexture*>& GetRenderTargets() const { return m_Targets; }

    void SetCurrentFrame(const float index) { m_Frame = index; }
    void SetCurrentFrameIndex(const uint32 index) { m_FrameIndex = index; }
    void SetCurrentImageIndex(const uint32 index) { m_ImageIndex = index; }
    void SetTarget(IRHITexture* target) { m_Target = target; }
    void SetRenderTargets(std::vector<IRHITexture*>& targets) { m_Targets = targets; }

protected:
    FContextSpec m_Spec;

    TRef<IRHIDevice> m_Device;
    TRef<IRHISwapchain> m_Swapchain;
    // std::vector<TRef<CVulkanCommandList>> m_FrameCommandLists;

    float m_Frame;
    uint32 m_FrameIndex;
    bool m_IsFirstRenderingInFrame = true;

    uint32 m_ImageIndex;
    IRHITexture* m_Target;
    std::vector<IRHITexture*> m_Targets;
};
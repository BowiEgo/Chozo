#include "RHIDevice.h"

#include "RHIContext.h"
#include "RHIResource.h"

DEFINE_LOG_CATEGORY(LogRHIDevice);

IRHIDevice::IRHIDevice(const IRHIContext* ctx, const FDeviceSpecification& spec)
    : m_Spec(spec), m_Context(ctx), m_ImagePool(WeakRef<IRHIDevice>(this)),
      m_SamplerCache(WeakRef<IRHIDevice>(this)), m_SetLayoutCache(WeakRef<IRHIDevice>(this)),
      m_DescriptorSetCache(WeakRef<IRHIDevice>(this)) {}

IRHIDevice::~IRHIDevice() { CZ_LOG(LogRHIDevice, Trace, "RHIDevice destroying..."); }

void IRHIDevice::EnqueueCleanup(std::function<void()>&& func) {
    uint32 frameIndex = m_Context->GetCurrentFrameIndex();

    m_DeletionQueue.push_back({ func, frameIndex });
}

void IRHIDevice::TickDeferredDeletion(uint32 currentFrame) {
    WaitIdle();

    uint32 safeFrame =
        (currentFrame + m_Context->GetMaxFramesInFlight() - 1) % m_Context->GetMaxFramesInFlight();

    auto it = m_DeletionQueue.begin();
    while (it != m_DeletionQueue.end()) {
        if (it->RetireFrame + 3 == safeFrame) {
            it->CleanupFunc();
            it = m_DeletionQueue.erase(it);
        } else {
            ++it;
        }
    }

    m_DescriptorSetCache.Trim();
}

std::vector<TRef<IRHISetLayout>>
    IRHIDevice::CreateDescriptorSetLayout(const FRHIPipelineLayoutDescription& desc) {
    std::vector<TRef<IRHISetLayout>> result;

    uint32_t maxSet = 0;
    for (auto const& [setIndex, _] : desc.SetLayouts) {
        maxSet = std::max(maxSet, setIndex);
    }

    result.resize(maxSet + 1);

    for (uint32_t i = 0; i <= maxSet; ++i) {
        if (desc.SetLayouts.contains(i)) {
            auto rhiLayout = GetOrCreateLayout(desc.SetLayouts.at(i));
            result[i]      = rhiLayout;
        } else {
            result[i] = GetEmptySetLayout();
        }
    }

    return result;
}
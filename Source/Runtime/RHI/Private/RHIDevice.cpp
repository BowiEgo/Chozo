#include "RHIDevice.h"

#include "RHIContext.h"
#include "RHIResource.h"

DEFINE_LOG_CATEGORY(LogRHIDevice);

IRHIDevice::IRHIDevice(const IRHIContext* ctx, const FDeviceSpecification& spec)
    : m_Spec(spec), m_Context(ctx) {}

IRHIDevice::~IRHIDevice() { CZ_LOG(LogRHIDevice, Trace, "RHIDevice destroying..."); }

void IRHIDevice::EnqueueCleanup(std::function<void()>&& func) {
    uint32 frameIndex = m_Context->GetCurrentFrameIndex();

    m_DeletionQueue.push_back({ func, frameIndex });
}

void IRHIDevice::TickDeferredDeletion(uint32 currentFrame, uint32 maxFramesInFlight) {
    WaitIdle();

    uint32 safeFrame = (currentFrame + maxFramesInFlight - 1) % maxFramesInFlight;

    auto it = m_DeletionQueue.begin();
    while (it != m_DeletionQueue.end()) {
        if (it->RetireFrame == safeFrame) {
            it->CleanupFunc();
            it = m_DeletionQueue.erase(it);
        } else {
            ++it;
        }
    }
}
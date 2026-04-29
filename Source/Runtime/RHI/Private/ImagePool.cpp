#include "ImagePool.h"

#include "RHIDevice.h"

DEFINE_LOG_CATEGORY(LogImagePool);

CImagePool::~CImagePool() {
    // CZ_LOG(LogImagePool, Trace, "ImagePool destroying... Releasing {} images.",
    // m_AvailableImages.size());
}

IRHIImage* CImagePool::RequestImage(const FImageSpecification& spec, uint32_t frame) {
    for (auto it = m_AvailableImages.begin(); it != m_AvailableImages.end(); ++it) {
        if (!it->bInUse && it->Image->GetSpec() == spec) {
            it->bInUse        = true;
            it->LastUsedFrame = frame;
            return it->Image.get();
        }
    }

    // If no matching image is found, create a new physical image
    auto device = m_Device.lock();
    if (!device) {
        CZ_LOG(LogImagePool, Error, "Device is no longer valid!");
        return nullptr;
    }

    auto newImage  = device->CreateImage(spec);
    IRHIImage* ptr = newImage.get();
    m_AvailableImages.push_back({ std::move(newImage), frame, true });

    return ptr;
}

void CImagePool::ReturnImage(IRHIImage* image) {
    if (!image) return;

    // Add the image back to the pool with the current frame index
    for (auto it = m_AvailableImages.begin(); it != m_AvailableImages.end(); ++it) {
        if (it->Image.get() == image) {
            it->bInUse = false;
            return;
        }
    }
}

void CImagePool::Tick(uint32_t currentFrame) {
    auto it = m_AvailableImages.begin();
    while (it != m_AvailableImages.end()) {
        if (!it->bInUse && currentFrame - it->LastUsedFrame > m_MaxIdleFrames) {
            it->Image->Destroy();
            it = m_AvailableImages.erase(it);
        } else {
            ++it;
        }
    }
}

void CImagePool::Clear() {
    CZ_LOG(LogImagePool, Trace, "ImagePool clearing images.");

    for (auto& [image, lastUsedFrame, bInUse] : m_AvailableImages) {
        image->Destroy();
    }

    m_AvailableImages.clear();
}

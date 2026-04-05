#include "ImagePool.h"

#include "RHIDevice.h"

DEFINE_LOG_CATEGORY(LogImagePool);

CImagePool::~CImagePool() {
    // CZ_LOG(LogImagePool, Trace, "ImagePool destroying... Releasing {} images.",
    // m_AvailableImages.size());
}

TRef<IRHIImage> CImagePool::RequestImage(const FImageSpecification& spec) {
    for (auto it = m_AvailableImages.begin(); it != m_AvailableImages.end(); ++it) {
        if (it->Image->GetSpec() == spec) {
            TRef<IRHIImage> foundImage = it->Image;
            // m_AvailableImages.erase(it); // Remove from idle pool, now owned by RenderGraph
            return foundImage;
        }
    }

    // If no matching image is found, create a new physical image
    auto device = m_Device.lock();
    if (!device) {
        CZ_LOG(LogImagePool, Error, "Device is no longer valid!");
        return nullptr;
    }

    auto newImage = device->CreateImage(spec);
    m_AvailableImages.push_back({ newImage, 0 });

    return newImage;
}

TRef<IRHIImage> CImagePool::RequestPersistentImage(const FImageSpecification& spec) {
    for (auto it = m_PersistentImages.begin(); it != m_PersistentImages.end(); ++it) {
        if (it->Image->GetSpec() == spec) {
            TRef<IRHIImage> foundImage = it->Image;
            return foundImage;
        }
    }

    // If no matching image is found, create a new physical image
    auto device = m_Device.lock();
    if (!device) {
        CZ_LOG(LogImagePool, Error, "Device is no longer valid!");
        return nullptr;
    }

    auto newImage = device->CreateImage(spec);
    m_PersistentImages.push_back({ newImage, 0 });

    return newImage;
}

void CImagePool::ReleaseImage(TRef<IRHIImage> image) {
    if (!image) return;

    // Add the image back to the pool with the current frame index
    m_AvailableImages.push_back({ image, 0 });
}

void CImagePool::Tick(uint32_t currentFrame) {
    auto it = m_AvailableImages.begin();
    while (it != m_AvailableImages.end()) {
        if (currentFrame - it->LastUsedFrame > m_MaxIdleFrames) {
            it = m_AvailableImages.erase(it);
        } else {
            ++it;
        }
    }
}

void CImagePool::Clear() {
    CZ_LOG(LogImagePool, Trace, "ImagePool clearing images.");

    for (auto& [image, lastFrame] : m_PersistentImages) {
        image->Destroy();
    }

    m_PersistentImages.clear();
}

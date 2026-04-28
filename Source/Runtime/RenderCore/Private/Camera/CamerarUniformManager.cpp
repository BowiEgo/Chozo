#include "CameraUniformManager.h"

#include "Buffer.h"
#include "RHIAPI.h"

DEFINE_LOG_CATEGORY(LogCameraUniformManager);

CCameraUniformManager& CCameraUniformManager::Get() {
    static CCameraUniformManager instance;
    return instance;
}

void CCameraUniformManager::Initialize() {}

void CCameraUniformManager::Shutdown() { m_Cameras.clear(); }

void CCameraUniformManager::RegisterCamera(CCamera* camera) {
    std::lock_guard<std::mutex> lock(m_Mutex);

    for (auto& entry : m_Cameras) {
        if (entry.camera == camera) return;
    }

    FBufferSpecification spec;
    spec.Size       = sizeof(CameraData);
    spec.Usage      = EBufferUsage::UniformBuffer;
    spec.MemoryType = EMemoryType::HostVisible | EMemoryType::HostCoherent;
    spec.Name       = "CameraUniformBuffer";

    CameraEntry entry;
    entry.camera = camera;
    entry.buffer = IRHIAPI::CreateBuffer(spec);

    m_Cameras.push_back(std::move(entry));

    CZ_LOG(LogCameraUniformManager, Info, "Camera registered: {}", (void*)camera);
}

void CCameraUniformManager::UnregisterCamera(CCamera* camera) {
    std::lock_guard<std::mutex> lock(m_Mutex);

    auto it = std::remove_if(m_Cameras.begin(), m_Cameras.end(),
                             [camera](const CameraEntry& entry) { return entry.camera == camera; });

    if (it != m_Cameras.end()) {
        m_Cameras.erase(it, m_Cameras.end());
        CZ_LOG(LogCameraUniformManager, Info, "Camera unregistered: {}", (void*)camera);
    }
}

void CCameraUniformManager::UpdateAllCameras() {
    std::lock_guard<std::mutex> lock(m_Mutex);

    for (auto& entry : m_Cameras) {
        entry.cachedData.view       = entry.camera->GetViewMatrix();
        entry.cachedData.projection = entry.camera->GetProjectionMatrix();

        // CZ_LOG(LogCameraUniformManager, Trace, "View matrix: \n{}",
        //        entry.cachedData.view.ToString().c_str());
        // CZ_LOG(LogCameraUniformManager, Trace, "Projection matrix : \n{} ",
        //        entry.cachedData.projection.ToString().c_str());

        FBuffer updateData(&entry.cachedData, sizeof(CameraData));
        // Upload to GPU
        entry.buffer->SetData(updateData);
    }
}

TRef<IRHIBuffer> CCameraUniformManager::GetBufferForCamera(const CCamera* camera) {
    std::lock_guard<std::mutex> lock(m_Mutex);

    for (auto& entry : m_Cameras) {
        if (entry.camera == camera) {
            return entry.buffer;
        }
    }

    return nullptr;
}

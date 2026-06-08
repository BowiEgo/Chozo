#include <Runtime/RHI/RHIAPI.hpp>
#include <Runtime/RenderCore/Camera/CameraManager.hpp>

namespace CZ {

CameraManager& CameraManager::Get() {
    static CameraManager instance;
    return instance;
}

void CameraManager::Shutdown() {
    for (auto camera : m_Cameras) {
        Delete(camera.Camera);
        camera.Buffer.Destroy();
    }
    m_Cameras.clear();
}

void CameraManager::RegisterCamera(CameraObj* camera) {
    std::lock_guard<std::mutex> lock(m_Mutex);

    for (auto& entry : m_Cameras) {
        if (entry.Camera == camera) return;
    }

    GraphicsBufferSpecification spec;
    spec.Size       = sizeof(CameraData);
    spec.Usage      = BufferUsage::UniformBuffer;
    spec.MemoryType = MemoryType::HostVisible | MemoryType::HostCoherent;
    spec.Name       = "CameraUniformBuffer";

    CameraEntry entry;
    entry.Camera = camera;
    entry.Buffer = RHIAPI::Get()->CreateGraphicsBuffer(spec);

    m_Cameras.push_back(std::move(entry));

    CZ_RENDERCORE_LOG(Info, "Camera registered: {}", (void*)camera);
}

void CameraManager::UnregisterCamera(CameraObj* camera) {
    std::lock_guard<std::mutex> lock(m_Mutex);

    auto it = std::remove_if(m_Cameras.begin(), m_Cameras.end(),
                             [camera](const CameraEntry& entry) { return entry.Camera == camera; });

    if (it != m_Cameras.end()) {
        m_Cameras.erase(it, m_Cameras.end());
        CZ_RENDERCORE_LOG(Info, "Camera unregistered: {}", (void*)camera);
    }
}

void CameraManager::UpdateAllCameras() {
    std::lock_guard<std::mutex> lock(m_Mutex);

    for (auto& entry : m_Cameras) {
        entry.CachedData.View       = entry.Camera->GetViewMatrix();
        entry.CachedData.Projection = entry.Camera->GetProjectionMatrix();

        SafeBuffer updateData = SafeBuffer::Copy(&entry.CachedData, sizeof(CameraData));

        // Upload to GPU
        entry.Buffer->SetData(&updateData);
    }
}

GraphicsBuffer CameraManager::GetCameraBuffer(const CameraObj* camera) {
    std::lock_guard<std::mutex> lock(m_Mutex);

    for (auto& entry : m_Cameras) {
        if (entry.Camera == camera) {
            return entry.Buffer;
        }
    }

    return GraphicsBuffer(); // Return invalid buffer if not found
}

} // namespace CZ
#pragma once

#include <Core/Math/Matrix4.hpp>
#include <Runtime/RHI/GraphicsBuffer.hpp>
#include <Runtime/RenderCore/Camera/Camera.hpp>

namespace CZ {

struct CameraData {
    Matrix4 View;
    Matrix4 Projection;
};

class CameraManager {
public:
    static CameraManager& Get();

    void Shutdown();

    void RegisterCamera(CameraObj* camera);
    void UnregisterCamera(CameraObj* camera);

    void UpdateAllCameras();

    GraphicsBuffer GetCameraBuffer(const CameraObj* camera);

private:
    CameraManager()  = default;
    ~CameraManager() = default;

private:
    struct CameraEntry {
        CameraObj* Camera;
        GraphicsBuffer Buffer;
        CameraData CachedData;
    };

    std::vector<CameraEntry> m_Cameras;
    std::mutex m_Mutex;
};

} // namespace CZ
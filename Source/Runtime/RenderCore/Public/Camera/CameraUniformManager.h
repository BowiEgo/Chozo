#pragma once

#include "Camera.h"
#include "RHIBuffer.h"
#include "RHIContext.h"
#include "RenderCoreExport.h"

DECLARE_LOG_CATEGORY_EXTERN(LogCameraUniformManager, Info);

struct CameraData {
    FMatrix4 view;
    FMatrix4 projection;
};

class RENDER_CORE_API CCameraUniformManager {
public:
    static CCameraUniformManager& Get() {
        static CCameraUniformManager instance;
        return instance;
    }

    void Initialize();
    void Shutdown();

    void RegisterCamera(CCamera* camera);
    void UnregisterCamera(CCamera* camera);

    void UpdateAllCameras();

    TRef<IRHIBuffer> GetBufferForCamera(const CCamera* camera);

private:
    CCameraUniformManager()  = default;
    ~CCameraUniformManager() = default;

private:
    struct CameraEntry {
        CCamera* camera;
        TRef<IRHIBuffer> buffer;
        CameraData cachedData;
    };

    std::vector<CameraEntry> m_Cameras;
    std::mutex m_Mutex;
};

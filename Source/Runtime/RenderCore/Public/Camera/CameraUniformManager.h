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
    CCameraUniformManager(IRHIContext* context);
    ~CCameraUniformManager() = default;

    void UpdateCamera(const CCamera& camera);

    TRef<IRHIBuffer> GetBuffer() { return m_CameraBuffer; }

private:
    IRHIContext* m_Context;
    TRef<IRHIBuffer> m_CameraBuffer;
    CameraData m_CachedData;
};

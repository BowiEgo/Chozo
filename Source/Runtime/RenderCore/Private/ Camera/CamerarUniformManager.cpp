#include "CameraUniformManager.h"

#include "Buffer.h"
#include "RHIAPI.h"

DEFINE_LOG_CATEGORY(LogCameraUniformManager);

CCameraUniformManager::CCameraUniformManager(IRHIContext* context) : m_Context(context) {
    FBufferSpecification spec;
    spec.Size = sizeof(CameraData);
    spec.Usage = EBufferUsage::UniformBuffer;
    spec.MemoryType = EMemoryType::HostVisible | EMemoryType::HostCoherent; // update per frame

    m_CameraBuffer = IRHIAPI::CreateBuffer(context, spec);
}

void CCameraUniformManager::UpdateCamera(const CCamera& camera) {
    m_CachedData.view = camera.GetViewMatrix();
    m_CachedData.projection = camera.GetProjectionMatrix();

    // CZ_LOG(LogCameraUniformManager, Trace, "View matrix: \n{}",
    //        m_CachedData.view.ToString().c_str());
    // CZ_LOG(LogCameraUniformManager, Trace, "Projection matrix : \n{} ",
    //        m_CachedData.projection.ToString().c_str());

    // Upload to GPU
    FBuffer updateData(&m_CachedData, sizeof(CameraData));
    m_CameraBuffer->SetData(updateData);
}

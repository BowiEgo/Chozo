#pragma once

#include <Runtime/RenderCore/Camera/Camera.hpp>

namespace CZ {

class SceneCamera : public Camera {
public:
    using Camera::Camera;

    /// Creates a camera owned by the caller.
    static Scope<CameraObj> Create(float fovDegrees, float aspectRatio, float nearClip,
                                   float farClip) {
        return CZ_CREATE_SCOPE(MEMORY_USAGE_RENDER, CameraObj, fovDegrees, aspectRatio, nearClip,
                               farClip);
    }

    void SyncFrom(const SceneCamera& source) {
        (*this)->SetPosition(source->GetPosition());
        (*this)->SetRotation(source->GetRotation());
        (*this)->SetPerspective(source->GetFOV(), source->GetAspectRatio(), source->GetNearClip(),
                                source->GetFarClip());
    }
};

} // namespace CZ
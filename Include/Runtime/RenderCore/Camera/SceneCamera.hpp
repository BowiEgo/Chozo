#pragma once

#include <Runtime/RenderCore/Camera/Camera.hpp>

namespace CZ {

class SceneCamera : public Camera {
public:
    using Camera::Camera;

    static SceneCamera Create(float fovDegrees, float aspectRatio, float nearClip, float farClip) {
        auto obj =
            CZ_NEW(MEMORY_USAGE_RENDER, CameraObj, fovDegrees, aspectRatio, nearClip, farClip);
        return SceneCamera(obj);
    }

    void SyncFrom(const SceneCamera& source) {
        (*this)->SetPosition(source->GetPosition());
        (*this)->SetRotation(source->GetRotation());
        (*this)->SetPerspective(source->GetFOV(), source->GetAspectRatio(), source->GetNearClip(),
                                source->GetFarClip());
    }
};

} // namespace CZ
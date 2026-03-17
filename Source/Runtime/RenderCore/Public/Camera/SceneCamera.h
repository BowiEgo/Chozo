#pragma once

#include "Camera.h"

#include "RenderCoreExport.h"

class RENDER_CORE_API CSceneCamera : public CCamera {
public:
    using CCamera::CCamera;

    void SyncFrom(const TRef<CSceneCamera> source) {
        SetPosition(source->GetPosition());
        SetRotation(source->GetRotation());
        SetPerspective(source->GetFOV(), source->GetAspectRatio(), source->GetNearClip(),
                       source->GetFarClip());
    }
};
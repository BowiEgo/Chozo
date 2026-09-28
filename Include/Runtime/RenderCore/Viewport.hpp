#pragma once

#include <Core/Header/Handle.hpp>
#include <Core/Header/Types.h>
#include <Runtime/RHI/FrameBuffer.hpp>
#include <Runtime/RenderCore/Camera/SceneCamera.hpp>
#include <Runtime/RenderCore/Scene/Scene.hpp>

namespace CZ {

struct ViewportSpecification {
    std::string Name;
    uint32 Width = 1, Height = 1;
};

struct ViewportObj {
    ViewportObj(const ViewportSpecification& spec);
    ~ViewportObj() = default;

    void Resize(uint32_t width, uint32_t height);

    void SetScene(Scene scene) { m_Scene = scene; }

    Scene GetScene() const { return m_Scene; }
    SceneCamera GetCamera() const { return SceneCamera(m_Camera.get()); }
    FrameBuffer GetFrameBuffer() const { return m_FrameBuffer; }

    const std::string& GetName() const { return m_Spec.Name; }

    uint32_t GetWidth() const { return m_Spec.Width; }

    uint32_t GetHeight() const { return m_Spec.Height; }

    float GetAspectRatio() const { return (float)m_Spec.Width / m_Spec.Height; }

    void CreateFrameBuffer();

    ViewportSpecification m_Spec;

    Scene m_Scene;

    // Owned; handed out as `SceneCamera` views. The camera unregisters itself from
    // `CameraManager` on destruction.
    Scope<CameraObj> m_Camera;
    FrameBuffer m_FrameBuffer;
};

struct Viewport : Handle<struct ViewportObj> {
    static Viewport Create(const ViewportSpecification& spec) {
        return Viewport(CZ_NEW(MEMORY_USAGE_RUNTIME, ViewportObj, spec));
    }
};

} // namespace CZ

#pragma once
#include <string>
#include <utility>

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

    /// Replaces the scene owned by this viewport.
    void SetScene(Scope<SceneObj> scene) { m_Scene = std::move(scene); }

    Scene GetScene() const { return ViewAs<Scene>(m_Scene); }
    SceneCamera GetCamera() const { return SceneCamera(m_Camera.get()); }
    FrameBuffer GetFrameBuffer() const { return ViewAs<FrameBuffer>(m_FrameBuffer); }

    const std::string& GetName() const { return m_Spec.Name; }

    uint32_t GetWidth() const { return m_Spec.Width; }

    uint32_t GetHeight() const { return m_Spec.Height; }

    float GetAspectRatio() const { return (float)m_Spec.Width / m_Spec.Height; }

    void CreateFrameBuffer();

    ViewportSpecification m_Spec;

    // All owned; handed out as views. The camera unregisters itself from `CameraManager`
    // on destruction.
    Scope<SceneObj> m_Scene;
    Scope<CameraObj> m_Camera;
    Scope<FrameBufferObj> m_FrameBuffer;
};

struct Viewport : Handle<struct ViewportObj> {
    /// Creates a viewport owned by the caller.
    static Scope<ViewportObj> Create(const ViewportSpecification& spec) {
        return CZ_CREATE_SCOPE(MEMORY_USAGE_RUNTIME, ViewportObj, spec);
    }
};

} // namespace CZ

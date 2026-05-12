#pragma once

#include <Core/Event/AppEvent.h>
#include <Core/Memory/Memory.h>
#include <Runtime/RHI/Device.h>
#include <Runtime/RHI/GraphicsContext.h>
#include <Runtime/RenderCore/Renderer.h>
#include <Runtime/Window/Window.h>

namespace CZ {

class Engine {
public:
    Engine() {};
    ~Engine();

    bool Init(std::string& err);
    void Tick(float deltaTime);
    void Shutdown();
    bool OnEvent(Event& e);
    void OnWindowResize(WindowResizedEvent& e);

    // Renderer* GetRenderer() const { return m_Renderer.get(); }

private:
    Renderer m_Renderer;
};

} // namespace CZ

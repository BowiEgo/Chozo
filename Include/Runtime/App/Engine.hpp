#pragma once

#include <Core/Event/AppEvent.hpp>
#include <Core/Memory/Memory.hpp>
#include <Runtime/RHI/Device.hpp>
#include <Runtime/RHI/GraphicsContext.hpp>
#include <Runtime/RenderCore/MeshRegistry.hpp>
#include <Runtime/RenderCore/Renderer.hpp>
#include <Runtime/RenderCore/Shader.hpp>
#include <Runtime/RenderCore/ShaderRegistry.hpp>
#include <Runtime/Window/Window.hpp>

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

    Renderer GetRenderer() const { return m_Renderer; }

    GraphicsContext GetGraphicContext() const { return m_GraphicsContext; }

    Texture GetSwapchainFramebuffer(uint32 index) {
        return m_GraphicsContext->GetSwapchain()->GetColorAttachment(index);
    }

    ShaderRegistry* GetShaderRegistry() { return m_ShaderRegistry.get(); }

private:
    Renderer m_Renderer;
    GraphicsContext m_GraphicsContext;

    Scope<ShaderRegistry> m_ShaderRegistry;
    Scope<MeshRegistry> m_MeshRegistry;
};

} // namespace CZ

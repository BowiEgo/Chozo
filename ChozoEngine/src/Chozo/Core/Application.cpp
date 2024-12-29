#include "Application.h"

#include "Chozo/Renderer/Renderer.h"
#include "Chozo/Renderer/Renderer2D.h"

#include "Chozo/Core/Thread.h"

#include <GLFW/glfw3.h>

namespace Chozo {

    Application* Application::s_Instance = nullptr;

    Application::Application(const std::string& name)
    {
        CZ_CORE_ASSERT(!s_Instance, "Application already exists!");
        s_Instance = this;

        RenderCommand::SwitchAPI(RenderAPI::Type::OpenGL);

        m_Window = Window::Create(WindowProps(name));
        m_Window->SetEventCallback(CZ_BIND_EVENT_FN(OnEvent));

        m_Pool = Ref<Pool>::Create();

        Renderer2D::Init();
        Renderer::Init();
        // TODO: Move to Project
        m_AssetManager = Ref<EditorAssetManager>::Create();
        m_AssetManager->LoadAssetRegistry();

        m_ImGuiLayer = new ImGuiLayer();
        PushOverlay(m_ImGuiLayer);
    }

    Application::~Application()
    {
		Renderer::Shutdown();
    }

    void Application::PushLayer(Layer* layer)
    {
        m_LayerStack.PushLayer(layer);
        layer->OnAttach();
    }

    void Application::PushOverlay(Layer* overlay)
    {
        m_LayerStack.PushOverlay(overlay);
        overlay->OnAttach();
    }

    void Application::OnEvent(Event& e)
    {
        EventDispatcher dispatcher(e);
        dispatcher.Dispatch<WindowCloseEvent>(CZ_BIND_EVENT_FN(OnWindowClose));

        for (auto it = m_LayerStack.end(); it != m_LayerStack.begin();)
        {
            (*--it)->OnEvent(e);
            if (e.isHandled())
                break;
        }
    }

    void Application::Run()
    {
        while (m_Running)
        {
            const auto time = (float)glfwGetTime(); // Platform::GetTime()
            const TimeStep timeStep = time - m_LastFrameTime;
            m_LastFrameTime = time;

            Renderer::Begin();

            m_Pool->Update();

            for (Layer* layer : m_LayerStack)
                layer->OnUpdate(timeStep);

            // TODO: execute this stuff on render thread.
            m_ImGuiLayer->Begin();
            for (Layer* layer : m_LayerStack)
                layer->OnImGuiRender();
            m_ImGuiLayer->End();

            Renderer::End();

            m_Window->OnUpdate();
        }
    }

    void Application::Close()
    {
        m_Running = false;
    }

    bool Application::OnWindowClose(WindowCloseEvent &e)
    {
        m_Running = false;
        return false;
    }
}

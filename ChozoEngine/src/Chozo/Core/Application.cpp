#include "Application.h"

#include <regex>

#include "Input.h"

#include "Pool.h"
#include "Chozo/Renderer/Renderer.h"
#include "Chozo/Renderer/Renderer2D.h"
#include "Chozo/Renderer/SceneRenderer.h"

#include <GLFW/glfw3.h>

#define BIND_EVENT_FN(x) std::bind(&Application::x, this, std::placeholders::_1)

namespace Chozo {

    // TODO: Move to Project
    extern const fs::path g_AssetsPath = "../assets";

    // TODO: Remove
    extern const std::regex imagePattern(R"(\.(png|jpg|jpeg|hdr)$)", std::regex::icase);
    extern const std::regex hdrPattern(R"(\.(hdr)$)", std::regex::icase);
    extern const std::regex scenePattern(R"(\.(chozo)$)", std::regex::icase);

    Application* Application::s_Instance = nullptr;

    Application::Application(const std::string& name)
    {
        CZ_CORE_ASSERT(!s_Instance, "Application already exists!");
        s_Instance = this;

        m_Window = Window::Create(WindowProps(name));
        m_Window->SetEventCallback(BIND_EVENT_FN(OnEvent));

        Pool::Init();
        Renderer2D::Init();
        Renderer::Init();
        // TODO: Move to Project
        m_AssetManager = Ref<EditorAssetManager>::Create();
        CZ_CORE_WARN("m_AssetManager");
        m_AssetManager->LoadAssetRegistry();

        m_ImGuiLayer = new ImGuiLayer();
        PushOverlay(m_ImGuiLayer);
    }
    
    Application::~Application()
    {
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
        dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(OnWindowClose));

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
            float time = (float)glfwGetTime(); // Platform::GetTime()
            Timestep timestep = time - m_LastFrameTime;
            m_LastFrameTime = time;

            Renderer::Begin();

            Pool::Update();

            for (Layer* layer : m_LayerStack)
                layer->OnUpdate(timestep);

            // TODO: excute this stuff on render thread.
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
        SceneRenderer::Shutdown();
    }

    bool Application::OnWindowClose(WindowCloseEvent &e)
    {
        m_Running = false;
        return false;
    }
}

#pragma once

#include "Chozo/Core/Base.h"
#include "Chozo/Core/Timestep.h"
#include "Chozo/Core/Window.h"
#include "Chozo/Core/LayerStack.h"
#include "Chozo/Core/Pool.h"

#include "Chozo/Events/Event.h"
#include "Chozo/Events/ApplicationEvent.h"

#include "Chozo/ImGui/ImGuiLayer.h"

#include "Chozo/Asset/EditorAssetManager.h"

namespace Chozo
{
    class CHOZO_API Application
    {
    public:
        Application(const std::string& name);
        virtual ~Application();

        void Run();
        void Close();

        void OnEvent(Event& e);

        void PushLayer(Layer* layer);
        void PushOverlay(Layer* overlay);

        inline Window& GetWindow() { return *m_Window; }
        inline Ref<Pool>& GetPool() { return m_Pool; }
        inline ImGuiLayer& GetImGuiLayer() { return *m_ImGuiLayer; }
        inline static Application& Get() { return *s_Instance; }

        static inline Ref<EditorAssetManager> GetAssetManager() { return s_Instance->m_AssetManager; }
    private:
        bool OnWindowClose(WindowCloseEvent& e);
    private:
        std::unique_ptr<Window> m_Window;
        Ref<Pool> m_Pool;
        ImGuiLayer* m_ImGuiLayer;
        bool m_Running = true;
        LayerStack m_LayerStack;
        Timestep m_Timestep;
        float m_LastFrameTime = 0.0f;
    private:
        static Application* s_Instance;
		Ref<EditorAssetManager> m_AssetManager;
    };

    // To be defined in CLIENT
    Application *CreateApplication();
}
#pragma once

#include "czpch.h"
#include "Chozo/Core/Base.h"
#include "Chozo/Events/Event.h"
#include "Chozo/Renderer/GraphicsContext.h"

#include <GLFW/glfw3.h>

namespace Chozo {

    struct WindowProps
    {
        std::string Title;
        unsigned int Width;
        unsigned int Height;

        WindowProps(const std::string& title = "Chozo Engine",
                    unsigned int width = WINDOW_WIDTH,
                    unsigned int height = WINDOW_HEIGHT)
            : Title(title), Width(width), Height(height)
        {
        }
    };
    
    class Window
    {
    public:
        using EventCallbackFn = std::function<void(Event&)>;

        Window(const WindowProps& props);
        ~Window();

        void Init(const WindowProps& props);
        void Shutdown();
        void OnUpdate();

        unsigned int GetWidth() const { return m_Data.Width; }
        unsigned int GetHeight() const { return m_Data.Height; }

        // Window attributes
        void SetEventCallback(const EventCallbackFn& callback) { m_Data.EventCallback = callback; }
        void SetVSync(bool enabled);
        bool IsVSync() const;

        void* GetNativeWindow() const { return m_Window; }

        static Scope<Window> Create(const WindowProps& props = WindowProps());
    private:
        GLFWwindow* m_Window;
        GraphicsContext* m_Context;

        struct WindowData
        {
            std::string Title;
            unsigned int Width, Height;
            bool VSync;

            EventCallbackFn EventCallback;
        };

        WindowData m_Data;
    };
}
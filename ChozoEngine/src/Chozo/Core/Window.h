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

        explicit WindowProps(std::string  title = "Chozo Engine",
                    const unsigned int width = WINDOW_WIDTH,
                    const unsigned int height = WINDOW_HEIGHT)
            : Title(std::move(title)), Width(width), Height(height)
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

        [[nodiscard]] unsigned int GetWidth() const { return m_Data.Width; }
        [[nodiscard]] unsigned int GetHeight() const { return m_Data.Height; }

        // Window attributes
        void SetEventCallback(const EventCallbackFn& callback) { m_Data.EventCallback = callback; }
        void SetVSync(bool enabled);
        [[nodiscard]] bool IsVSync() const;

        [[nodiscard]] GLFWwindow* GetNativeWindow() const { return m_Window; }
        [[nodiscard]] GLFWwindow* GetSharedWindow() const { return m_SharedWindow; }

        static Scope<Window> Create(const WindowProps& props = WindowProps());
    private:
        GLFWwindow* m_Window{};
        GLFWwindow* m_SharedWindow{};
        GraphicsContext* m_Context{};

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
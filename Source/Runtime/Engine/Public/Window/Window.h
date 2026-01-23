#pragma once

namespace Chozo
{
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
        Window(const WindowProps& props);
        ~Window();

        void Init(const WindowProps& props);
        void Shutdown();
        void OnUpdate();
    private:
        GLFWwindow* m_Window{};
        GLFWwindow* m_SharedWindow{};
        // GraphicsContext* m_Context{};

        struct WindowData
        {
            std::string Title;
            float XScale, YScale = 1.0f;
            unsigned int Width, Height;
            float PixelRatio;
            bool VSync;
        };

        WindowData m_Data;
    };
}
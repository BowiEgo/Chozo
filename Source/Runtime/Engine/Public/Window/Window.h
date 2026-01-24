#pragma once

#include "WindowDefinition.h"

namespace Chozo
{

    class ENGINE_API FWindow
    {
    public:
        FWindow(const FWindowDefinition& windowDef);
        ~FWindow();

        void Init(const FWindowDefinition& windowDef);
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
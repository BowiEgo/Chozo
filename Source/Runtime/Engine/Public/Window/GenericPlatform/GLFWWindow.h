#pragma once

#include "Core.h"
#include "Window.h"

namespace Chozo
{

    DECLARE_LOG_CATEGORY_EXTERN(GLFWWindow, Info);

    class ENGINE_API GLFWWindow : public FWindow
    {
    public:
        GLFWWindow(const FWindowDefinition& windowDef);
        ~GLFWWindow();

        void Init(const FWindowDefinition& windowDef);
        void Shutdown();
        void OnUpdate();
    private:
        GLFWwindow* m_Window{};
        GLFWwindow* m_SharedWindow{};
        // GraphicsContext* m_Context{};

        WindowData m_Data;
    };
}
#include "Window.h"
#include "Backend/OpenGL/OpenGLContext.h"

namespace Chozo {
    
    static bool s_GLFWInitialized = false;

    static void GLFWErrorCallback(int error, const char* description)
    {
        CZ_CORE_ERROR("GLFW Error ({0}):", error, description);
    }

    Scope<Window> Chozo::Window::Create(const WindowProps &props)
    {
        return std::make_unique<Window>(props);
    }

    Window::Window(const WindowProps& props)
    {
        Init(props);
    }

    Window::~Window()
    {
    }

    void Window::Init(const WindowProps& props)
    {
        m_Data.Title = props.Title;
        m_Data.Width = props.Width;
        m_Data.Height = props.Height;

        CZ_CORE_INFO("Creating window {0} ({1}, {2})", props.Title, props.Width, props.Height);

        // Initialize GLFW
        if (!s_GLFWInitialized)
        {
            int sucess = glfwInit();
            CZ_CORE_ASSERT(sucess, "Could not initialize GLFW!");
            glfwSetErrorCallback(GLFWErrorCallback);
            s_GLFWInitialized = true;
        }

        // Set GLFW window hints for OpenGL version and profile
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        #ifdef __APPLE__
            glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        #endif

        // Create a GLFW window
        m_Window = glfwCreateWindow((int)props.Width, (int)props.Height, m_Data.Title.c_str(), nullptr, nullptr);

        m_Context = new OpenGLContext(m_Window);
        m_Context->Init();
        
        glfwSetWindowUserPointer(m_Window, &m_Data);
        SetVSync(false);

        // Set GLFW callbacks
        glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* window, int width, int height)
        {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
            data.Width = width;
            data.Height = height;

            WindowResizedEvent event(width, height);
            data.EventCallback(event);
        });

        glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* window)
        {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

            WindowCloseEvent event;
            data.EventCallback(event);
        });

        glfwSetKeyCallback(m_Window, [](GLFWwindow* window, int key, int scancode, int action, int mods)
        {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

            switch (action)
            {
                case GLFW_PRESS:
                {
                    KeyPressedEvent event((KeyCode)key, 0);
                    data.EventCallback(event);
                    break;
                }
                case GLFW_RELEASE:
                {
                    KeyReleasedEvent event((KeyCode)key);
                    data.EventCallback(event);
                    break;
                }
                case GLFW_REPEAT:
                {
                    KeyPressedEvent event((KeyCode)key, 1);
                    data.EventCallback(event);
                    break;
                }
            }
        });

        glfwSetCharCallback(m_Window, [](GLFWwindow* window, uint32_t character)
        {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
			KeyTypedEvent event((KeyCode)character);
            data.EventCallback(event);
        });

        glfwSetMouseButtonCallback(m_Window, [](GLFWwindow* window, int button, int action, int mods)
        {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

            switch (action)
            {
                case GLFW_PRESS:
                {
                    MouseButtonPressedEvent event((Button)button);
                    data.EventCallback(event);
                    break;
                }
                case GLFW_RELEASE:
                {
                    MouseButtonReleasedEvent event((Button)button);
                    data.EventCallback(event);
                    break;
                }
            }
        });

        glfwSetScrollCallback(m_Window, [](GLFWwindow* window, double xOffset, double yOffset)
        {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

            MouseScrolledEvent event((float)xOffset, (float)yOffset);
            data.EventCallback(event);
        });

        glfwSetCursorPosCallback(m_Window, [](GLFWwindow* window, double xPos, double yPos)
        {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

            MouseMovedEvent event((float)xPos, (float)yPos);
            data.EventCallback(event);
        });
    }

    void Window::Shutdown()
    {
        std::cout << "close" << std::endl;
        glfwDestroyWindow(m_Window);
    }

    void Window::OnUpdate()
    {
        /* Swap front and back buffers */
        m_Context->SwapBuffers();

        /* Poll for and process events */
        glfwPollEvents();
    }

    void Window::SetVSync(bool enabled)
    {
        if (enabled)
            glfwSwapInterval(1);
        else
            glfwSwapInterval(0);

        m_Data.VSync = enabled;
    }

    bool Window::IsVSync() const
    {
        return m_Data.VSync;
    }
}
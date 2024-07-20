#include "MacInput.h"

#include "Chozo/Core/Application.h"
#include <GLFW/glfw3.h>

namespace Chozo {

    Input* Input::s_Instance = new MacInput();

    // bool Chozo::MacInput::IsKeyPressedImpl(int keycode)
    // {
    //     auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
    //     auto state = glfwGetKey(window, keycode);

    //     return state == GLFW_PRESS || state == GLFW_REPEAT;
    // }

    bool Chozo::MacInput::IsKeyPressedImpl(KeyCode keycode)
    {
        auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
        auto state = glfwGetKey(window, static_cast<int32_t>(keycode));

        return state == GLFW_PRESS || state == GLFW_REPEAT;
    }

    // bool Chozo::MacInput::IsMouseButtonPressedImpl(int button)
    // {
    //     auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
    //     auto state = glfwGetKey(window, button);

    //     return state == GLFW_PRESS;
    // }

    bool Chozo::MacInput::IsMouseButtonPressedImpl(MouseButton button)
    {
        auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
        auto state = glfwGetMouseButton(window, static_cast<int32_t>(button));

        return state == GLFW_PRESS;
    }

    std::pair<float, float> MacInput::GetMousePositionImpl()
    {
        auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);

        return { (float)xpos, (float)ypos };
    }

    float Chozo::MacInput::GetMouseXImpl()
    {
        auto [x, y] = GetMousePositionImpl();

        return x;
    }

    float Chozo::MacInput::GetMouseYImpl()
    {
        auto [x, y] = GetMousePositionImpl();

        return y;
    }
}


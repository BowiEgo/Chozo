#include "Chozo/Core/Input.h"

#include "Chozo/Core/Application.h"
#include <GLFW/glfw3.h>

namespace Chozo {

    Input* Input::s_Instance = new Input();

    // bool Input::IsKeyPressedImpl(int keycode)
    // {
    //     auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
    //     auto state = glfwGetKey(window, keycode);

    //     return state == GLFW_PRESS || state == GLFW_REPEAT;
    // }

    bool Input::IsKeyPressedImpl(KeyCode keycode)
    {
        const auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
        const auto state = glfwGetKey(window, static_cast<int32_t>(keycode));

        return state == GLFW_PRESS || state == GLFW_REPEAT;
    }

    // bool Input::IsMouseButtonPressedImpl(int button)
    // {
    //     auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
    //     auto state = glfwGetKey(window, button);

    //     return state == GLFW_PRESS;
    // }

    bool Input::IsMouseButtonPressedImpl(MouseButton button)
    {
        const auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
        const auto state = glfwGetMouseButton(window, static_cast<int32_t>(button));

        return state == GLFW_PRESS;
    }

    std::pair<float, float> Input::GetMousePositionImpl()
    {
        const auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
        double xPos, yPos;
        glfwGetCursorPos(window, &xPos, &yPos);

        return { (float)xPos, (float)yPos };
    }

    float Input::GetMouseXImpl()
    {
        auto [x, y] = GetMousePositionImpl();

        return x;
    }

    float Input::GetMouseYImpl()
    {
        auto [x, y] = GetMousePositionImpl();

        return y;
    }
}


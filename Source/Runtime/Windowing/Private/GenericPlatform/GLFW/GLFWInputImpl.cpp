#include "Input.h"

#include "GLFWWindow.h"

#include <GLFW/glfw3.h>

SInput* SInput::s_Instance = new SInput();

void SInput::Init(CWindow* window) { s_Instance->m_Window = window; }

bool SInput::IsKeyPressedImpl(EKeyCode keycode) {
    const auto state = glfwGetKey(static_cast<CGLFWWindow*>(s_Instance->m_Window)->GetGLFWWindow(),
                                  static_cast<int32>(keycode));
    return state == GLFW_PRESS || state == GLFW_REPEAT;
}

bool SInput::IsMouseButtonPressedImpl(EMouseButton button) {
    const auto state =
        glfwGetMouseButton(static_cast<CGLFWWindow*>(s_Instance->m_Window)->GetGLFWWindow(),
                           static_cast<int32>(button));
    return state == GLFW_PRESS;
}

std::pair<float, float> SInput::GetMousePositionImpl() {
    double xPos, yPos;
    glfwGetCursorPos(static_cast<CGLFWWindow*>(s_Instance->m_Window)->GetGLFWWindow(), &xPos,
                     &yPos);

    return { (float)xPos, (float)yPos };
}

float SInput::GetMouseXImpl() {
    auto [x, y] = GetMousePositionImpl();

    return x;
}

float SInput::GetMouseYImpl() {
    auto [x, y] = GetMousePositionImpl();

    return y;
}

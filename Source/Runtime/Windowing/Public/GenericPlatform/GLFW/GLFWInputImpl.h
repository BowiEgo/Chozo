#include "InputImpl.h"

#include "GLFWWindow.h"

#include <GLFW/glfw3.h>

class CGLFWInputImpl : public IInputImpl {
public:
    CGLFWInputImpl(CWindow* window) : m_Window(window) {}

    virtual bool IsKeyPressed(EKeyCode keycode) override {
        const auto state = glfwGetKey(static_cast<CGLFWWindow*>(m_Window)->GetGLFWWindow(),
                                      static_cast<int32>(keycode));
        return state == GLFW_PRESS || state == GLFW_REPEAT;
    }

    virtual bool IsMouseButtonPressed(EMouseButton button) override {
        const auto state = glfwGetMouseButton(static_cast<CGLFWWindow*>(m_Window)->GetGLFWWindow(),
                                              static_cast<int32>(button));
        return state == GLFW_PRESS;
    }

    virtual std::pair<float, float> GetMousePosition() override {
        double xPos, yPos;
        glfwGetCursorPos(static_cast<CGLFWWindow*>(m_Window)->GetGLFWWindow(), &xPos, &yPos);

        return { (float)xPos, (float)yPos };
    }

    virtual float GetMouseX() override {
        auto [x, y] = GetMousePosition();

        return x;
    }

    virtual float GetMouseY() override {
        auto [x, y] = GetMousePosition();

        return y;
    }

private:
    CWindow* m_Window;
};
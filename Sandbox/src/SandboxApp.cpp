#include "Parallax.h"

class ExampleLayer : public Parallax::Layer
{
public:
    ExampleLayer()
        : Layer("Example")
    {
    }

    void OnUpdate() override
    {
        if (Parallax::Input::IsKeyPressed(PRX_KEY_TAB))
            PRX_TRACE("Tab key is pressed (poll)!");
    }

    void OnEvent(Parallax::Event& event) override
    {
        if (event.GetEventType() == Parallax::EventType::KeyPressed)
        {
            Parallax::KeyPressedEvent& e = (Parallax::KeyPressedEvent&)event;
            if (e.GetKeyCode() == PRX_KEY_TAB)
                PRX_TRACE("Tab key is pressed (event)!");
            PRX_TRACE("{0}", (char)e.GetKeyCode());
        }
    }
};

class Sandbox : public Parallax::Application
{
public:
    Sandbox()
    {
        PRX_INFO("Welcome to Parallax Sandbox!");
        PushLayer(new ExampleLayer());
        PushOverlay(new Parallax::ImGuiLayer());
    }

    ~Sandbox()
    {
    }
};

Parallax::Application *Parallax::CreateApplication()
{
    return new Sandbox();
};

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
        PRX_INFO("ExampleLayer::Update");
    }

    void OnEvent(Parallax::Event& event) override
    {
        PRX_TRACE("{0}", event);
    }
};

class Sandbox : public Parallax::Application
{
public:
    Sandbox()
    {
        PRX_INFO("Welcome to Parallax Sandbox!");
        PushLayer(new ExampleLayer());
    }

    ~Sandbox()
    {
    }
};

Parallax::Application *Parallax::CreateApplication()
{
    return new Sandbox();
};

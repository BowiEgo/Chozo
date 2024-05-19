#include "Parallax.h"

#include <iostream>

class Sandbox : public Parallax::Application
{
public:
    Sandbox()
    {
        PX_INFO("Welcome to Parallax Sandbox!");
    }

    ~Sandbox()
    {
    }
};

Parallax::Application *Parallax::CreateApplication()
{
    return new Sandbox();
};

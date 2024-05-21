#include "Parallax.h"

#include <iostream>

class Sandbox : public Parallax::Application
{
public:
    Sandbox()
    {
        PRX_INFO("Welcome to Parallax Sandbox!");
    }

    ~Sandbox()
    {
    }
};

Parallax::Application *Parallax::CreateApplication()
{
    return new Sandbox();
};

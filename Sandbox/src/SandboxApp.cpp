#include "Parallax.h"
#include <iostream>

class Sandbox : public Parallax::Application
{
public:
    Sandbox()
    {
        std::cout << "Welcome to Parallax Sandbox!" << std::endl;
    }

    ~Sandbox()
    {

    }
};

Parallax::Application* Parallax::CreateApplication()
{
    return new Sandbox();
};

#pragma once

#include "Application.h"

int main(int argc, char** argv)
{
    auto app = Parallax::CreateApplication();
    app->Run();
    delete app;
}
#pragma once

#include "Application.h"

int main(int argc, char **argv)
{
    Parallax::Log::Init();
    PX_CORE_WARN("Initialzed Log!");
    int a = 50;
    PX_CORE_INFO("Hello! Var={0}", a);

    auto app = Parallax::CreateApplication();
    app->Run();
    delete app;
}
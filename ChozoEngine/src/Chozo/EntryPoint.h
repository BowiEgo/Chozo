#pragma once

#include "Chozo/Core/Application.h"

int main(int argc, char **argv)
{
    Chozo::Log::Init();
    CZ_CORE_WARN("Initialzed Log!");
    int a = 50;
    CZ_CORE_INFO("Hello! Var={0}", a);

    auto app = Chozo::CreateApplication();
    app->Run();
    delete app;
}
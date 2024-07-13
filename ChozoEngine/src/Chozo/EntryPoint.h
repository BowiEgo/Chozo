#pragma once

#include "Chozo/Core/Application.h"

int main(int argc, char **argv)
{
    Chozo::Log::Init();
    CZ_CORE_WARN("Log system initialzed!");

    auto app = Chozo::CreateApplication();
    app->Run();
    delete app;
}
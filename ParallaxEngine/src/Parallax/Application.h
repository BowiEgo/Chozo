#pragma once

#include "prxpch.h"
#include "Core.h"
#include "Events/Event.h"

namespace Parallax
{
    class PARALLAX_API Application
    {
    public:
        Application();
        virtual ~Application();

        void Run();
    };

    // To be defined in CLIENT
    Application *CreateApplication();
}
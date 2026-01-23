#pragma once

namespace Chozo
{
    class EngineLoop
    {
    public:
        EngineLoop();
        virtual ~EngineLoop() {};

        static void Init();
        static void Tick();
        static void Exit();
    };
}
#pragma once

#include "Core.h"

namespace Chozo {

    DECLARE_LOG_CATEGORY_EXTERN(LogEngineLoop, Info);

    class ENGINE_API FEngineLoop
    {
    public:
        FEngineLoop();
        virtual ~FEngineLoop() {};

        static void Init();
        static void Tick();
        static void Exit();
    };
}

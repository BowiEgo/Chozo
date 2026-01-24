#pragma once

#include "Core.h"
#include "Window.h"

namespace Chozo {

    DECLARE_LOG_CATEGORY_EXTERN(LogEngineLoop, Info);

    class ENGINE_API FEngineLoop
    {
    public:
        FEngineLoop();
        virtual ~FEngineLoop() {};

        void Init();
        void Tick();
        void Exit();

        bool ShouldClose() const;
    private:
        std::unique_ptr<FWindow> m_Window;
    };
}

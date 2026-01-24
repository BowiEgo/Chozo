#include "FEngineLoop.h"

namespace Chozo {

    DEFINE_LOG_CATEGORY(LogEngineLoop);

    FEngineLoop::FEngineLoop()
    { }

    void FEngineLoop::Init()
	{
        CZ_LOG(LogEngineLoop, Info, "Engine Loop Initializing...");
	}

    void FEngineLoop::Tick()
    {
    }

    void FEngineLoop::Exit()
    {
    }
}

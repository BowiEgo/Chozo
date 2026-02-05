#pragma once

#include "Core.h"
#include "EngineExport.h"
#include "RenderEngine.h"
#include "Window.h"

DECLARE_LOG_CATEGORY_EXTERN(LogEngineLoop, Info);

class ENGINE_API CEngineLoop {
public:
    CEngineLoop();
    virtual ~CEngineLoop() {};

    void Init();
    void Tick();
    void Exit();

    bool ShouldClose() const;

private:
    TScope<CWindow> m_Window;
    TScope<CRenderEngine> m_RenderEngine;
};

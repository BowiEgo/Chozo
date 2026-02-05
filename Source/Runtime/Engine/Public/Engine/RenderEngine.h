#pragma once

#include "Core.h"
#include "EngineExport.h"
#include "GraphicsContext.h"
#include "RHIDevice.h"
#include "Window.h"

DECLARE_LOG_CATEGORY_EXTERN(LogRenderEngine, Info);

class ENGINE_API CRenderEngine {
public:
    CRenderEngine(CWindow* window) : m_Window(window) {};
    ~CRenderEngine() = default;

    void Init();

private:
    CWindow* m_Window;
    TRef<IRHIDevice> m_Device;
};

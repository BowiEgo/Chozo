#pragma once

#include "WindowDefinition.h"

namespace Chozo
{

    class ENGINE_API FWindow
    {
    public:
        FWindow(const FWindowDefinition& windowDef);
        ~FWindow();

        virtual void Init(const FWindowDefinition& windowDef);
        virtual void Shutdown();
        virtual void OnUpdate();
    protected:
        struct WindowData
        {
            std::string Title;
            float XScale, YScale = 1.0f;
            unsigned int Width, Height;
            float PixelRatio;
            bool VSync;
        };
    
        WindowData m_Data;
    };
}
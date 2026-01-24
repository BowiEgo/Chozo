#pragma once

#include "WindowDefinition.h"

namespace Chozo
{

    class ENGINE_API FWindow
    {
    public:
        FWindow(const FWindowDefinition& windowDef);
        ~FWindow() {};

        void Init(const FWindowDefinition& windowDef);
        virtual void Shutdown() = 0;
        virtual void OnUpdate() = 0;
        virtual void SetVSync(bool enabled) = 0;
        virtual bool ShouldClose() const = 0;

        unsigned int GetWidth() const { return m_Data.Width; }
        unsigned int GetHeight() const { return m_Data.Height; }
        float GetPixelRatio() const { return m_Data.PixelRatio; }
        bool IsVSync() const { return m_Data.VSync; }
        float GetDPI() const { return m_Data.XScale; }
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
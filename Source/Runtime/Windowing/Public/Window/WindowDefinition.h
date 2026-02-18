#pragma once

#include "WindowingExport.h"

struct WINDOWING_API FWindowDefinition {
    std::string Title;
    float XScale, YScale = 1.0f;
    unsigned int Width, Height;
    float PixelRatio;
    bool VSync;
    FEventCallback EventCallback;

    FWindowDefinition() : Title("Chozo Engine"), Width(1280), Height(720) {}
};

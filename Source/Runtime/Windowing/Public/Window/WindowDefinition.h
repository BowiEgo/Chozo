#pragma once

#include "WindowingExport.h"

struct WINDOWING_API FWindowDefinition {
    std::string Title;
    float XScale, YScale = 1.0f;
    FExtent2D Size;
    float PixelRatio;
    bool VSync;
    FEventCallback EventCallback;

    FWindowDefinition() : Title("Chozo Engine"), Size({ 1280, 720 }) {}
};

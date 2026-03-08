#pragma once

#include "CoreMinimal.h"
#include "UIExport.h"

enum class EOverlayLocation {
    TopLeft = 0, // Default position, matches your current FPS counter.
    TopRight,    // Suitable for system status or clock.
    BottomLeft,  // Often used for coordinates or logs.
    BottomRight, // Standard location for notifications or gizmo info.
    Center,      // Used for critical alerts or loading indicators.
    Unknown
};

class UI_API UOverlay {
public:
    UOverlay();
    ~UOverlay();

    void Draw(const char* title, bool* bIsOpen, const std::function<void()>& contentRenderFn);
    void UpdateLocation(EOverlayLocation Location);

private:
    int m_LocationIndex = 0;
    ImVec2 m_LastSize = { 320.0f, 100.0f };
};
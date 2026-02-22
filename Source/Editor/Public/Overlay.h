#pragma once

#include "CoreMinimal.h"

enum class EOverlayLocation {
    TopLeft = 0, // Default position, matches your current FPS counter.
    TopRight,    // Suitable for system status or clock.
    BottomLeft,  // Often used for coordinates or logs.
    BottomRight, // Standard location for notifications or gizmo info.
    Center,      // Used for critical alerts or loading indicators.
    Unknown
};

class Overlay {
public:
    Overlay();
    ~Overlay();

    void Draw(bool* bIsOpen, const std::function<void()>& contentRenderFn);
    void UpdateLocation(EOverlayLocation Location);

private:
    int m_LocationIndex = 0;
};
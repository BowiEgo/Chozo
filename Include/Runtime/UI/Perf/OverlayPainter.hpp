#pragma once
#include <cstddef>
#include <string_view>

namespace CZ {

/// Backend-independent colour (linear 0..1, alpha included).
struct OverlayColor {
    float R = 1.0f, G = 1.0f, B = 1.0f, A = 1.0f;
};

/// The drawing surface the perf overlay lays itself out on.
///
/// The overlay's layout code only knows these primitives, so swapping ImGui for the engine's own
/// 2D/text renderer later means implementing this interface once -- no changes to PerfOverlay.
/// Primitives are deliberately coarse (text, separator, plot, panel) because that is all the
/// overlay needs; richer widgets belong to the panel that owns them.
class OverlayPainter {
public:
    virtual ~OverlayPainter() = default;

    /// Opens a translucent panel whose top-left corner is at (x, y) in viewport space.
    virtual void BeginPanel(std::string_view id, float x, float y, float width) = 0;
    virtual void EndPanel()                                                     = 0;

    virtual void Text(std::string_view text, const OverlayColor& color) = 0;
    virtual void Separator()                                            = 0;
    /// Simple line graph; values are clamped to [minValue, maxValue] by the implementation.
    virtual void Plot(std::string_view label, const float* values, size_t count, float minValue,
                      float maxValue, const OverlayColor& color)        = 0;
};

} // namespace CZ

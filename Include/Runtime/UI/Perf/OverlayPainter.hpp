#pragma once
#include <cstddef>
#include <string_view>

namespace CZ {

/// Backend-independent colour (0..1, alpha included).
struct OverlayColor {
    float R = 1.0f, G = 1.0f, B = 1.0f, A = 1.0f;
};

/// The drawing surface the perf overlay lays itself out on.
///
/// The overlay's layout code only knows these primitives, so swapping ImGui for the engine's own
/// 2D/text renderer later means implementing this interface once -- no changes to PerfOverlay.
///
/// Text measurement and two-column rows live here rather than in the layout because both need real
/// font metrics, which only the backend has. Keeping them as primitives is what lets PerfOverlay
/// stay free of any drawing API while still producing aligned labels and right-aligned values.
class OverlayPainter {
public:
    virtual ~OverlayPainter() = default;

    /// Opens a panel whose top-left corner is at (x, y) and whose content box is `width` wide.
    virtual void BeginPanel(std::string_view id, float x, float y, float width) = 0;
    virtual void EndPanel()                                                     = 0;

    virtual void Text(std::string_view text, const OverlayColor& color) = 0;
    /// Label and value on one line: label flush left, value flush right inside the panel.
    virtual void Row(std::string_view label, std::string_view value, const OverlayColor& labelColor,
                     const OverlayColor& valueColor)                    = 0;
    virtual void Separator()                                            = 0;
    /// Simple line graph; the implementation clamps values into [minValue, maxValue].
    virtual void Plot(std::string_view label, const float* values, size_t count, float minValue,
                      float maxValue, const OverlayColor& color)        = 0;

    /// Width of `text` in pixels, used for auto sizing and right alignment.
    virtual float MeasureText(std::string_view text) = 0;
};

} // namespace CZ

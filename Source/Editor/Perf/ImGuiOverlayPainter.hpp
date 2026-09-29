#pragma once
#include <Runtime/UI/Perf/OverlayPainter.hpp>

namespace CZ {

/// ImGui implementation of the overlay primitives. This is the *only* file in the overlay stack
/// that depends on ImGui, which is what keeps PerfOverlay portable to the engine's own 2D/text
/// renderer later.
class ImGuiOverlayPainter final : public OverlayPainter {
public:
    void BeginPanel(std::string_view id, float x, float y, float width) override;
    void EndPanel() override;
    void Text(std::string_view text, const OverlayColor& color) override;
    void Separator() override;
    void Plot(std::string_view label, const float* values, size_t count, float minValue,
              float maxValue, const OverlayColor& color) override;
};

} // namespace CZ

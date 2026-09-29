#pragma once
#include <Runtime/UI/Perf/OverlayPainter.hpp>

namespace CZ {

/// ImGui implementation of the overlay primitives, painted on the *foreground* draw list.
///
/// The background is drawn through a separate draw channel so it can be emitted after the text
/// while still rendering behind it. Keeping the state as plain floats leaves this header free of
/// ImGui types, so only the translation unit below depends on ImGui.
class ImGuiOverlayPainter final : public OverlayPainter {
public:
    void BeginPanel(std::string_view id, float x, float y, float width) override;
    void EndPanel() override;
    void Text(std::string_view text, const OverlayColor& color) override;
    void Separator() override;
    void Plot(std::string_view label, const float* values, size_t count, float minValue,
              float maxValue, const OverlayColor& color) override;

private:
    float m_X = 0.0f, m_Y = 0.0f, m_Width = 0.0f;
    float m_CursorY = 0.0f, m_Height = 0.0f;
    bool m_Active = false;
};

} // namespace CZ

#pragma once
#include <Runtime/UI/Perf/OverlayPainter.hpp>
#include <string_view>

namespace CZ {

/// ImGui implementation of the overlay primitives, painted on the *foreground* draw list.
///
/// The background goes through a second draw channel so it can be emitted after the text while
/// still compositing behind it. All state is plain floats, so this header stays free of ImGui types
/// and only the translation unit below depends on ImGui.
class ImGuiOverlayPainter final : public OverlayPainter {
public:
    void BeginPanel(std::string_view id, float x, float y, float width) override;
    void EndPanel() override;
    void Text(std::string_view text, const OverlayColor& color) override;
    void Row(std::string_view label, std::string_view value, const OverlayColor& labelColor,
             const OverlayColor& valueColor) override;
    void Separator() override;
    void Plot(std::string_view label, const float* values, size_t count, float minValue,
              float maxValue, const OverlayColor& color) override;
    float MeasureText(std::string_view text) override;

private:
    void Advance(float height);

    float m_X = 0.0f, m_Y = 0.0f, m_Width = 0.0f;
    float m_CursorY = 0.0f, m_Height = 0.0f;
    bool m_Active = false;
};

} // namespace CZ

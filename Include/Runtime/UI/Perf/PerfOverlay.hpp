#pragma once
#include <cstddef>

#include <Runtime/UI/Perf/FrameStats.hpp>
#include <Runtime/UI/Perf/OverlayPainter.hpp>

namespace CZ {

/// Viewport performance overlay: owns the frame history and lays it out through an OverlayPainter.
///
/// Three layers, each replaceable on its own:
///   FrameStats     -- measurement and rolling aggregates (no drawing)
///   OverlayPainter -- drawing primitives, including text measurement (ImGui today, engine 2D
///   later) PerfOverlay    -- rows, colours, wording and column sizing (this class)
class PerfOverlay {
public:
    void PushFrame(double frameSeconds, const PhaseSample* phases, size_t phaseCount);

    FrameStats& Stats() { return m_Stats; }
    const FrameStats& Stats() const { return m_Stats; }

    void SetVisible(bool bVisible) { m_Visible = bVisible; }
    bool Toggle() {
        m_Visible = !m_Visible;
        return m_Visible;
    }
    bool IsVisible() const { return m_Visible; }

    /// Paints the overlay at (x, y). The panel grows to fit its widest row, never beyond maxWidth.
    void Draw(OverlayPainter& painter, float x, float y, float maxWidth) const;

private:
    FrameStats m_Stats;
    bool m_Visible = true;
};

} // namespace CZ

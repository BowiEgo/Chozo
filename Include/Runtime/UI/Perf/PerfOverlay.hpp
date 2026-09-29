#pragma once
#include <cstddef>

#include <Runtime/UI/Perf/FrameStats.hpp>
#include <Runtime/UI/Perf/OverlayPainter.hpp>

namespace CZ {

/// Viewport performance overlay: owns the frame history and lays it out through an OverlayPainter.
///
/// Deliberately split in three layers so each can change independently:
///   FrameStats    -- measurement and rolling aggregates (no drawing at all)
///   OverlayPainter-- drawing primitives (ImGui today, engine 2D/text later)
///   PerfOverlay   -- layout, colours and wording (this class)
class PerfOverlay {
public:
    void PushFrame(double frameSeconds, const PhaseSample* phases, size_t phaseCount);

    FrameStats& Stats() { return m_Stats; }

    /// GPU timings (may be empty when unsupported).
    void SetGpuPhases(const PhaseSample* phases, size_t count) {
        m_Stats.SetGpuPhases(phases, count);
    }
    const FrameStats& Stats() const { return m_Stats; }

    void SetVisible(bool bVisible) { m_Visible = bVisible; }
    bool Toggle() {
        m_Visible = !m_Visible;
        return m_Visible;
    }
    bool IsVisible() const { return m_Visible; }

    /// Paints the overlay; does nothing while hidden or before the first sample.
    void Draw(OverlayPainter& painter, float x, float y, float width) const;

private:
    FrameStats m_Stats;
    bool m_Visible = true;
};

} // namespace CZ

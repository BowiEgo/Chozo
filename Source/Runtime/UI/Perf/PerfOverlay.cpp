#include <Runtime/UI/Perf/PerfOverlay.hpp>

#include <fmt/format.h>

namespace CZ {
namespace {

constexpr OverlayColor kNeutral{ 0.85f, 0.85f, 0.88f, 1.0f };
constexpr OverlayColor kDim{ 0.62f, 0.62f, 0.66f, 1.0f };
constexpr OverlayColor kGood{ 0.45f, 0.85f, 0.50f, 1.0f };
constexpr OverlayColor kWarn{ 0.95f, 0.80f, 0.35f, 1.0f };
constexpr OverlayColor kOver{ 0.95f, 0.40f, 0.40f, 1.0f };

OverlayColor ColorFor(FrameBudgetStatus status) {
    switch (status) {
        case FrameBudgetStatus::Good: return kGood;
        case FrameBudgetStatus::Warning: return kWarn;
        default: return kOver;
    }
}

} // namespace

void PerfOverlay::PushFrame(double frameSeconds, const PhaseSample* phases, size_t phaseCount) {
    m_Stats.Push(frameSeconds, phases, phaseCount);
}

void PerfOverlay::Draw(OverlayPainter& painter, float x, float y, float width) const {
    if (!m_Visible || m_Stats.SampleCount() == 0) {
        return;
    }

    const double frame = m_Stats.Smoothed();
    const double worst = m_Stats.WorstonePercent();

    painter.BeginPanel("##PerfOverlay", x, y, width);
    painter.Text(fmt::format("{:.2f} ms   {:.0f} FPS", frame * 1000.0, m_Stats.Fps()),
                 ColorFor(m_Stats.StatusOf(frame)));
    painter.Text(
        fmt::format("1% low {:.2f} ms   max {:.2f} ms", worst * 1000.0, m_Stats.Max() * 1000.0),
        kDim);
    painter.Plot("##PerfGraph", m_Stats.History(), FrameStats::HistorySize(), 0.0f,
                 static_cast<float>(m_Stats.TargetSeconds() * 1000.0 * 3.0), kNeutral);
    painter.Separator();

    for (size_t i = 0; i < m_Stats.PhaseCount(); ++i) {
        const PhaseSample& phase = m_Stats.Phase(i);
        painter.Text(fmt::format("{:<18}{:>6.2f} ms", phase.Name, phase.Seconds * 1000.0),
                     ColorFor(m_Stats.StatusOf(phase.Seconds)));
    }

    if (m_Stats.DrawCalls() > 0) {
        painter.Text(fmt::format("{:<18}{:>6} draws / {} tris", "Draws", m_Stats.DrawCalls(),
                                 m_Stats.Triangles()),
                     kDim);
    }

    painter.EndPanel();
}

} // namespace CZ

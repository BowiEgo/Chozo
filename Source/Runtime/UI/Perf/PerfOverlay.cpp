#include <Runtime/UI/Perf/PerfOverlay.hpp>

#include <algorithm>
#include <string>
#include <vector>

#include <fmt/format.h>

namespace CZ {
namespace {

constexpr OverlayColor kNeutral{ 0.85f, 0.85f, 0.88f, 1.0f };
constexpr OverlayColor kDim{ 0.62f, 0.62f, 0.66f, 1.0f };
constexpr OverlayColor kGood{ 0.45f, 0.85f, 0.50f, 1.0f };
constexpr OverlayColor kWarn{ 0.95f, 0.80f, 0.35f, 1.0f };
constexpr OverlayColor kOver{ 0.95f, 0.40f, 0.40f, 1.0f };

constexpr float kPadding  = 16.0f; ///< left + right padding of the content box
constexpr float kMinWidth = 150.0f;

OverlayColor ColorFor(FrameBudgetStatus status) {
    switch (status) {
        case FrameBudgetStatus::Good: return kGood;
        case FrameBudgetStatus::Warning: return kWarn;
        default: return kOver;
    }
}

/// One line of the overlay: an empty label means a plain left-aligned line (the title line).
struct Line {
    std::string Label;
    std::string Value;
    OverlayColor Color = kNeutral;
    bool bSeparator    = false;
    bool bPlot         = false;
};

} // namespace

void PerfOverlay::PushFrame(double frameSeconds, const PhaseSample* phases, size_t phaseCount) {
    m_Stats.Push(frameSeconds, phases, phaseCount);
}

void PerfOverlay::Draw(OverlayPainter& painter, float x, float y, float maxWidth) const {
    if (!m_Visible || m_Stats.SampleCount() == 0) {
        return;
    }

    const double frame = m_Stats.Smoothed();

    // === Content first, so the panel width can be measured before it is opened ===
    std::vector<Line> lines;
    lines.push_back({ "", fmt::format("{:.2f} ms    {:.0f} FPS", frame * 1000.0, m_Stats.Fps()),
                      ColorFor(m_Stats.StatusOf(frame)), false, false });
    lines.push_back({ "1% low", fmt::format("{:.2f} ms", m_Stats.WorstonePercent() * 1000.0), kDim,
                      false, false });
    lines.push_back(
        { "max", fmt::format("{:.2f} ms", m_Stats.Max() * 1000.0), kDim, false, false });
    lines.push_back({ "", "", kDim, true, false });
    lines.push_back({ "", "", kNeutral, false, true });

    for (size_t i = 0; i < m_Stats.PhaseCount(); ++i) {
        const PhaseSample& phase = m_Stats.Phase(i);
        lines.push_back({ std::string(phase.Name), fmt::format("{:.2f} ms", phase.Seconds * 1000.0),
                          ColorFor(m_Stats.StatusOf(phase.Seconds)), false, false });
    }

    if (m_Stats.GpuPhaseCount() > 0) {
        lines.push_back({ "", "", kDim, true, false });
        for (size_t i = 0; i < m_Stats.GpuPhaseCount(); ++i) {
            const PhaseSample& gpu = m_Stats.GpuPhase(i);
            lines.push_back({ std::string(gpu.Name), fmt::format("{:.2f} ms", gpu.Seconds * 1000.0),
                              ColorFor(m_Stats.StatusOf(gpu.Seconds)), false, false });
        }
    }

    if (m_Stats.DrawCalls() > 0) {
        lines.push_back({ "", "", kDim, true, false });
        lines.push_back({ "draws",
                          fmt::format("{}  ({} tris, {} switches)", m_Stats.DrawCalls(),
                                      m_Stats.Triangles(), m_Stats.PipelineSwitches()),
                          kDim, false, false });
    }

    float labelWidth = 0.0f;
    float valueWidth = 0.0f;
    for (const Line& line : lines) {
        labelWidth = std::max(labelWidth, painter.MeasureText(line.Label));
        valueWidth = std::max(valueWidth, painter.MeasureText(line.Value));
    }

    const float width =
        std::clamp(labelWidth + valueWidth + kPadding, kMinWidth, std::max(kMinWidth, maxWidth));

    painter.BeginPanel("##PerfOverlay", x, y, width);
    for (const Line& line : lines) {
        if (line.bSeparator) {
            painter.Separator();
        } else if (line.bPlot) {
            m_Graph.resize(FrameStats::HistorySize());
            m_Stats.CopyHistoryOldestFirst(m_Graph.data());
            painter.Plot("##PerfGraph", m_Graph.data(), m_Graph.size(), 0.0f,
                         static_cast<float>(m_Stats.TargetSeconds() * 1000.0 * 3.0), kNeutral);
        } else if (line.Label.empty()) {
            painter.Text(line.Value, line.Color);
        } else {
            painter.Row(line.Label, line.Value, kDim, line.Color);
        }
    }
    painter.EndPanel();
}

} // namespace CZ

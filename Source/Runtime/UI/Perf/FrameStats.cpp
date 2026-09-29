#include <Runtime/UI/Perf/FrameStats.hpp>

#include <algorithm>

namespace CZ {

void FrameStats::Push(double frameSeconds, const PhaseSample* phases, size_t phaseCount) {
    m_FrameSeconds[m_Head] = frameSeconds;
    m_History[m_Head]      = static_cast<float>(frameSeconds * 1000.0);
    m_Head                 = (m_Head + 1) % kHistory;
    m_Count                = std::min(m_Count + 1, kHistory);

    m_PhaseCount = std::min(phaseCount, kMaxPhases);
    for (size_t i = 0; i < m_PhaseCount; ++i) {
        m_Phases[i] = phases[i];
    }

    // Cheap exponential smoothing: enough for an overlay, no allocation, no sorting.
    constexpr double kAlpha = 0.1;
    m_Smoothed = m_Count == 1 ? frameSeconds : m_Smoothed * (1.0 - kAlpha) + frameSeconds * kAlpha;
}

void FrameStats::CopyHistoryOldestFirst(float* out) const {
    const size_t oldest = (m_Head + kHistory - m_Count) % kHistory;
    for (size_t i = 0; i < m_Count; ++i) {
        out[i] = m_History[(oldest + i) % kHistory];
    }
    for (size_t i = m_Count; i < kHistory; ++i) {
        out[i] = 0.0f; // not enough samples yet: draw a flat line rather than stale data
    }
}

void FrameStats::SetGpuPhases(const PhaseSample* phases, size_t count) {
    m_GpuPhaseCount = std::min(count, kMaxPhases);
    for (size_t i = 0; i < m_GpuPhaseCount; ++i) {
        m_GpuPhases[i] = phases[i];
    }
}

void FrameStats::Clear() {
    m_History.fill(0.0f);
    m_FrameSeconds.fill(0.0);
    m_Count = m_Head = m_PhaseCount = 0;
    m_GpuPhaseCount                 = 0;
    m_DrawCalls = m_Triangles = 0;
    m_Smoothed                = 0.0;
}

double FrameStats::Average() const {
    if (m_Count == 0) return 0.0;
    double sum = 0.0;
    for (size_t i = 0; i < m_Count; ++i)
        sum += m_FrameSeconds[i];
    return sum / static_cast<double>(m_Count);
}

double FrameStats::WorstonePercent() const {
    if (m_Count == 0) return 0.0;
    size_t take = std::max<size_t>(1, m_Count / 100);
    std::array<double, kHistory> sorted{};
    std::copy_n(m_FrameSeconds.begin(), m_Count, sorted.begin());
    std::sort(sorted.begin(), sorted.begin() + static_cast<ptrdiff_t>(m_Count),
              std::greater<double>());
    double sum = 0.0;
    for (size_t i = 0; i < take; ++i)
        sum += sorted[i];
    return sum / static_cast<double>(take);
}

double FrameStats::Max() const {
    if (m_Count == 0) return 0.0;
    return *std::max_element(m_FrameSeconds.begin(),
                             m_FrameSeconds.begin() + static_cast<ptrdiff_t>(m_Count));
}

double FrameStats::Fps() const {
    const double smoothed = m_Smoothed > 0.0 ? m_Smoothed : Current();
    return smoothed > 0.0 ? 1.0 / smoothed : 0.0;
}

FrameBudgetStatus FrameStats::StatusOf(double seconds) const {
    if (seconds <= m_Target) return FrameBudgetStatus::Good;
    if (seconds <= m_Target * 2.0) return FrameBudgetStatus::Warning;
    return FrameBudgetStatus::Over;
}

} // namespace CZ

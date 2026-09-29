#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>

namespace CZ {

/// Which budget a measurement is inside of. The overlay only colours text by this, so the policy
/// lives here rather than in the painter.
enum class FrameBudgetStatus : uint8_t { Good, Warning, Over };

/// One named CPU phase of the frame (mirrors the Tracy zone names so the overlay and the profiler
/// can be read side by side).
struct PhaseSample {
    std::string_view Name;
    float Seconds = 0.0f;
};

/// Rolling frame statistics. Pure data: it neither knows about ImGui nor about the engine's
/// renderer, so a future engine-side 2D painter can render the very same numbers.
class FrameStats {
public:
    static constexpr size_t kHistory   = 120; ///< ~2s at 60 FPS
    static constexpr size_t kMaxPhases = 8;

    /// Records one frame. Phase entries beyond kMaxPhases are ignored.
    void Push(double frameSeconds, const PhaseSample* phases, size_t phaseCount);
    void Clear();

    // === Rolling aggregates (seconds) ===
    double Current() const { return m_FrameSeconds.back(); }
    double Smoothed() const { return m_Smoothed; } ///< exponential moving average
    double Average() const;                        ///< mean over the window
    double WorstonePercent() const;                ///< mean of the slowest 1%
    double Max() const;
    double Fps() const;

    FrameBudgetStatus StatusOf(double seconds) const;

    size_t SampleCount() const { return m_Count; }
    const float* History() const { return m_History.data(); }
    static constexpr size_t HistorySize() { return kHistory; }

    /// GPU timings arrive a few frames late, so they are kept on their own channel.
    void SetGpuPhases(const PhaseSample* phases, size_t count);
    size_t GpuPhaseCount() const { return m_GpuPhaseCount; }
    const PhaseSample& GpuPhase(size_t index) const { return m_GpuPhases[index]; }

    size_t PhaseCount() const { return m_PhaseCount; }
    const PhaseSample& Phase(size_t index) const { return m_Phases[index]; }

    /// Target frame time used for the budget colouring.
    void SetTargetSeconds(double seconds) { m_Target = seconds; }
    double TargetSeconds() const { return m_Target; }

private:
    std::array<float, kHistory> m_History{}; ///< ms, for the graph
    std::array<double, kHistory> m_FrameSeconds{};
    std::array<PhaseSample, kMaxPhases> m_Phases{};
    std::array<PhaseSample, kMaxPhases> m_GpuPhases{};
    size_t m_GpuPhaseCount = 0;
    size_t m_Count         = 0;
    size_t m_Head          = 0;
    size_t m_PhaseCount    = 0;
    double m_Smoothed      = 0.0;
    double m_Target        = 1.0 / 60.0;
};

} // namespace CZ

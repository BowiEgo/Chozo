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

    /// Draw calls and triangles of the last completed frame.
    void SetDrawCalls(uint32_t draws, uint32_t triangles, uint32_t pipelineSwitches = 0) {
        m_DrawCalls        = draws;
        m_Triangles        = triangles;
        m_PipelineSwitches = pipelineSwitches;
    }
    uint32_t DrawCalls() const { return m_DrawCalls; }
    uint32_t Triangles() const { return m_Triangles; }
    uint32_t PipelineSwitches() const { return m_PipelineSwitches; }

    /// GPU timings arrive a few frames late, so they live on their own channel.
    void SetGpuPhases(const PhaseSample* phases, size_t count);
    size_t GpuPhaseCount() const { return m_GpuPhaseCount; }
    const PhaseSample& GpuPhase(size_t index) const { return m_GpuPhases[index]; }

    size_t SampleCount() const { return m_Count; }
    /// Copies the frame-time history (milliseconds) oldest first into `out`, which must hold
    /// HistorySize() floats. The ring buffer is stored rotated, so plotting it directly makes the
    /// graph jump instead of scrolling; exporting in chronological order keeps new samples on the
    /// right edge.
    void CopyHistoryOldestFirst(float* out) const;
    static constexpr size_t HistorySize() { return kHistory; }

    size_t PhaseCount() const { return m_PhaseCount; }
    const PhaseSample& Phase(size_t index) const { return m_Phases[index]; }

    /// Target frame time used for the budget colouring.
    void SetTargetSeconds(double seconds) { m_Target = seconds; }
    double TargetSeconds() const { return m_Target; }

private:
    std::array<float, kHistory> m_History{}; ///< ms, for the graph
    std::array<double, kHistory> m_FrameSeconds{};
    std::array<PhaseSample, kMaxPhases> m_Phases{};
    size_t m_Count      = 0;
    size_t m_Head       = 0;
    size_t m_PhaseCount = 0;
    std::array<PhaseSample, kMaxPhases> m_GpuPhases{};
    size_t m_GpuPhaseCount      = 0;
    uint32_t m_DrawCalls        = 0;
    uint32_t m_Triangles        = 0;
    uint32_t m_PipelineSwitches = 0;
    double m_Smoothed           = 0.0;
    double m_Target             = 1.0 / 60.0;
};

} // namespace CZ

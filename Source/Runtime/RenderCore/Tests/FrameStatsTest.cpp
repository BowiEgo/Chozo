// Rolling frame statistics: the overlay's data layer, independent of any drawing backend.
#include <Runtime/UI/Perf/FrameStats.hpp>

#include <doctest/doctest.h>

using namespace CZ;

namespace {
void PushN(FrameStats& stats, size_t count, double seconds) {
    for (size_t i = 0; i < count; ++i) {
        stats.Push(seconds, nullptr, 0);
    }
}
} // namespace

TEST_CASE("Frame stats report frame time, FPS and budget status") {
    FrameStats stats;
    PushN(stats, 10, 1.0 / 60.0);

    CHECK(stats.SampleCount() == 10);
    CHECK(stats.Current() == doctest::Approx(1.0 / 60.0));
    CHECK(stats.Fps() == doctest::Approx(60.0).epsilon(0.05));
    CHECK(stats.StatusOf(1.0 / 60.0) == FrameBudgetStatus::Good);
    CHECK(stats.StatusOf(1.0 / 30.0) == FrameBudgetStatus::Warning);
    CHECK(stats.StatusOf(0.5) == FrameBudgetStatus::Over);
}

TEST_CASE("The history is exported oldest first so graphs scroll instead of rotating") {
    FrameStats stats;
    for (int i = 0; i < 5; ++i) {
        stats.Push(static_cast<double>(i + 1) / 1000.0, nullptr, 0);
    }

    float history[FrameStats::HistorySize()] = {};
    stats.CopyHistoryOldestFirst(history);

    CHECK(history[0] == doctest::Approx(1.0f));
    CHECK(history[4] == doctest::Approx(5.0f));
}

TEST_CASE("Wrapping the ring buffer keeps the chronological order") {
    FrameStats stats;
    PushN(stats, FrameStats::HistorySize(), 0.010);
    stats.Push(0.020, nullptr, 0);

    float history[FrameStats::HistorySize()] = {};
    stats.CopyHistoryOldestFirst(history);

    CHECK(history[FrameStats::HistorySize() - 1] == doctest::Approx(20.0f)); // newest on the right
    CHECK(history[FrameStats::HistorySize() - 2] == doctest::Approx(10.0f));
}

TEST_CASE("The 1% low averages the slowest samples only") {
    FrameStats stats;
    PushN(stats, 99, 0.010);
    stats.Push(0.100, nullptr, 0);

    CHECK(stats.WorstonePercent() == doctest::Approx(0.100));
    CHECK(stats.Max() == doctest::Approx(0.100));
}

TEST_CASE("CPU phases, GPU phases and draw counters are stored independently") {
    FrameStats stats;
    const PhaseSample phases[2] = { { "Layer Update", 0.001f }, { "Frame - Update", 0.015f } };
    stats.Push(0.016, phases, 2);

    CHECK(stats.PhaseCount() == 2);
    CHECK(stats.Phase(0).Name == "Layer Update");
    CHECK(stats.GpuPhaseCount() == 0); // GPU timings arrive late and use their own channel

    const PhaseSample gpu[2] = { { "GPU Scene", 0.009f }, { "GPU UI", 0.001f } };
    stats.SetGpuPhases(gpu, 2);
    CHECK(stats.GpuPhaseCount() == 2);
    CHECK(stats.GpuPhase(0).Seconds == doctest::Approx(0.009f));

    stats.SetDrawCalls(5, 4800, 7);
    CHECK(stats.DrawCalls() == 5);
    CHECK(stats.Triangles() == 4800);
    CHECK(stats.PipelineSwitches() == 7);

    stats.Clear();
    CHECK(stats.SampleCount() == 0);
    CHECK(stats.GpuPhaseCount() == 0);
    CHECK(stats.DrawCalls() == 0);
    CHECK(stats.PipelineSwitches() == 0);
}

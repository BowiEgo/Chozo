#pragma once

#include "LogMacros.h"
#include "Logger.h"

class Timer {
public:
    Timer() { Reset(); }

    void Reset() { m_Start = std::chrono::high_resolution_clock::now(); }

    float Elapsed() const {
        return std::chrono::duration_cast<std::chrono::microseconds>( // NOLINT
                   std::chrono::high_resolution_clock::now() - m_Start)
                   .count() *
               0.001f * 0.001f;
    }

    float ElapsedMillis() const {
        return std::chrono::duration_cast<std::chrono::microseconds>( // NOLINT
                   std::chrono::high_resolution_clock::now() - m_Start)
                   .count() *
               0.001f;
    }

    static void Wait(float targetMillis) {
        Timer waitTimer;

        // Yield the thread if the remaining time is large enough.
        // On Windows, sleep(1) is usually the smallest reliable gap.
        while (targetMillis - waitTimer.ElapsedMillis() > 1.5f) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }

        // Busy-wait for the last micro-seconds to get exact precision.
        while (waitTimer.ElapsedMillis() < targetMillis) {
            // Do nothing, just loop until time is up.
            std::this_thread::yield();
        }
    }

private:
    std::chrono::time_point<std::chrono::high_resolution_clock> m_Start;
};

DECLARE_LOG_CATEGORY_EXTERN(LogScopedTimer, Info);

class ScopedTimer {
public:
    explicit ScopedTimer(std::string name) : m_Name(std::move(name)) {}
    ~ScopedTimer() {
        float time = m_Timer.ElapsedMillis();
        CZ_LOG(LogScopedTimer, Trace, "TIMER", "{0} - {1}ms", m_Name, time);
    }

private:
    std::string m_Name;
    Timer m_Timer;
};

enum class EProfileSlot : uint32_t {
    TotalFrame = 0,
    Logic,
    ImGui,
    Render,
    Wait,
    COUNT // Used for array sizing
};

// Keep human-readable names for UI display only
const char* const GProfileSlotNames[] = { "Total FrameTime", "LogicUpdate", "ImGui Render",
                                          "Render", "Wait Time" };

static_assert(sizeof(GProfileSlotNames) / sizeof(const char*) ==
                  static_cast<uint32_t>(EProfileSlot::COUNT),
              "Profiler slot names count mismatch!");

class PerformanceProfiler {
    struct SlotData {
        float Time       = 0.0f;
        uint32_t Samples = 0;

        SlotData() = default;

        SlotData(const float time) : Time(time) {} // NOLINT

        explicit operator float() const { return Time; }
        inline SlotData& operator+=(const float time) {
            Time += time;
            return *this;
        }

        void Reset() {
            Time    = 0.0f;
            Samples = 0;
        }
    };

public:
    void SetTiming(EProfileSlot slot, const float time) {
        uint32_t index = static_cast<uint32_t>(slot);
        if (index < static_cast<uint32_t>(EProfileSlot::COUNT)) {
            m_Buffers[m_WriteIndex][index].Time += time;
            m_Buffers[m_WriteIndex][index].Samples++;
        }
    }

    void Flip() {
        uint32_t readIndex = m_WriteIndex;
        m_WriteIndex       = 1 - m_WriteIndex; // Toggle between 0 and 1

        for (uint32_t i = 0; i < static_cast<uint32_t>(EProfileSlot::COUNT); ++i) {
            const SlotData& data = m_Buffers[readIndex][i];
            float currentAvg     = (data.Samples > 0) ? (data.Time / data.Samples) : 0.0f;

            if (m_SmoothedValues[i] == 0.0f) {
                m_SmoothedValues[i] = currentAvg;
            } else {
                m_SmoothedValues[i] =
                    m_SmoothingAlpha * currentAvg + (1.0f - m_SmoothingAlpha) * m_SmoothedValues[i];
            }
        }

        for (auto& slot : m_Buffers[m_WriteIndex]) {
            slot.Reset();
        }
    }

    const SlotData& GetSlot(EProfileSlot slot) const {
        uint32_t index = static_cast<uint32_t>(slot);
        return m_Buffers[1 - m_WriteIndex][index];
    }

    float GetRawAverage(EProfileSlot slot) const {
        uint32_t index       = static_cast<uint32_t>(slot);
        const SlotData& data = m_Buffers[1 - m_WriteIndex][index];
        return (data.Samples > 0) ? (data.Time / data.Samples) : 0.0f;
    }

    float GetSmoothedAverage(EProfileSlot slot) const {
        uint32_t index = static_cast<uint32_t>(slot);
        return m_SmoothedValues[index];
    }

    void SetSmoothingFactor(float alpha) { m_SmoothingAlpha = alpha; }

private:
    uint32_t m_WriteIndex = 0;
    std::array<SlotData, static_cast<uint32_t>(EProfileSlot::COUNT)> m_Buffers[2];

    float m_SmoothingAlpha = 0.1f;
    std::array<float, static_cast<uint32_t>(EProfileSlot::COUNT)> m_SmoothedValues;
};

class ScopePerfTimer {
public:
    ScopePerfTimer(const EProfileSlot slot, PerformanceProfiler* profiler)
        : m_Slot(slot), m_Profiler(profiler) {}

    ~ScopePerfTimer() {
        float time = m_Timer.ElapsedMillis();
        m_Profiler->SetTiming(m_Slot, time);
    }

private:
    EProfileSlot m_Slot;
    PerformanceProfiler* m_Profiler;
    Timer m_Timer;
};

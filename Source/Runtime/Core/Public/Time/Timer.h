#pragma once

#include "LogMacros.h"
#include "Logger.h"

#include <unordered_map>

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

namespace ProfilerKeys {
inline const std::string TotalFrame = "Total FrameTime";
inline const std::string Render = "RenderPass";
inline const std::string Logic = "LogicUpdate";
inline const std::string Wait = "Wait Time";
} // namespace ProfilerKeys

class PerformanceProfiler {
    struct PerFrameData {
        float Time = 0.0f;
        uint32_t Samples = 0;

        PerFrameData() = default;

        PerFrameData(const float time) : Time(time) {} // NOLINT

        explicit operator float() const { return Time; }
        inline PerFrameData& operator+=(const float time) {
            Time += time;
            return *this;
        }
    };

    using ProfilerMap = std::unordered_map<std::string, PerFrameData>;

public:
    void SetPerFrameTiming(const std::string& key, const float time) {
        std::scoped_lock<std::mutex> lock(m_PerFrameDataMutex);

        // operator[] will value-initialize PerFrameData if key doesn't exist
        auto& data = m_PerFrameData[key];
        data.Time += time;
        data.Samples++;
    }

    /**
     * @brief Encapsulated lookup method.
     * @return Pointer to data if found, nullptr otherwise.
     * * Using pointer return to avoid copying large structures.
     */
    const PerFrameData* GetEntry(const std::string& key) const {
        // No lock here if you only call this from the main UI thread
        // after all Ticks are done. Otherwise, keep the lock.
        auto it = m_PerFrameData.find(key);
        return (it != m_PerFrameData.end()) ? &it->second : nullptr;
    }

    void Clear() {
        std::scoped_lock<std::mutex> lock(m_PerFrameDataMutex);
        m_PerFrameData.clear();
    }

    [[nodiscard]] const ProfilerMap& GetPerFrameData() const { return m_PerFrameData; }

private:
    ProfilerMap m_PerFrameData;
    mutable std::mutex m_PerFrameDataMutex;
};

class ScopePerfTimer {
public:
    ScopePerfTimer(const char* name, PerformanceProfiler* profiler)
        : m_Name(name), m_Profiler(profiler) {}

    ~ScopePerfTimer() {
        float time = m_Timer.ElapsedMillis();
        m_Profiler->SetPerFrameTiming(m_Name, time);
    }

private:
    const char* m_Name;
    PerformanceProfiler* m_Profiler;
    Timer m_Timer;
};

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

class PerformanceProfiler {
public:
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

public:
    void SetPerFrameTiming(const char* name, const float time) {
        std::scoped_lock<std::mutex> lock(m_PerFrameDataMutex);

        if (m_PerFrameData.find(name) == m_PerFrameData.end()) m_PerFrameData[name] = 0.0f;

        PerFrameData& data = m_PerFrameData[name];
        data.Time += time;
        data.Samples++;
    }

    void Clear() {
        std::scoped_lock<std::mutex> lock(m_PerFrameDataMutex);
        m_PerFrameData.clear();
    }

    [[nodiscard]] const std::unordered_map<const char*, PerFrameData>& GetPerFrameData() const {
        return m_PerFrameData;
    }

private:
    std::unordered_map<const char*, PerFrameData> m_PerFrameData;
    inline static std::mutex m_PerFrameDataMutex;
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

// #if 1
//     #define CZ_SCOPE_PERF(name) \
//         ScopePerfTimer timer__LINE__(name, CApplication::Get().GetPerformanceProfiler());

//     #define CZ_SCOPE_TIMER(name) ScopedTimer timer__LINE__(name);
// #else
//     #define CZ_SCOPE_PERF(name)
//     #define CZ_SCOPE_TIMER(name)
// #endif
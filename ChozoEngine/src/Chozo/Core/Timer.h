#pragma once

#include <unordered_map>

#include "Chozo/Debug/Log.h"

namespace Chozo {

    class Timer
	{
	public:
		CZ_FORCE_INLINE Timer() { Reset(); }
		CZ_FORCE_INLINE void Reset() { m_Start = std::chrono::high_resolution_clock::now(); }
		CZ_FORCE_INLINE float Elapsed() const {
			return std::chrono::duration_cast<std::chrono::microseconds>( // NOLINT
				       std::chrono::high_resolution_clock::now() - m_Start).count() * 0.001f * 0.001f;
		}
		CZ_FORCE_INLINE float ElapsedMillis() const {
			return std::chrono::duration_cast<std::chrono::microseconds>( // NOLINT
				       std::chrono::high_resolution_clock::now() - m_Start).count() * 0.001f;
		}
	private:
		std::chrono::time_point<std::chrono::high_resolution_clock> m_Start;
	};
	
	class ScopedTimer
	{
	public:
		explicit ScopedTimer(std::string  name)
			: m_Name(std::move(name)) {}
		~ScopedTimer()
		{
			float time = m_Timer.ElapsedMillis();
			CZ_CORE_TRACE_TAG("TIMER", "{0} - {1}ms", m_Name, time);
		}
	private:
		std::string m_Name;
		Timer m_Timer;
	};

	class PerformanceProfiler
	{
	public:
		struct PerFrameData
		{
			float Time = 0.0f;
			uint32_t Samples = 0;

			PerFrameData() = default;

			PerFrameData(const float time) : Time(time) {} // NOLINT

			explicit operator float() const { return Time; }
			inline PerFrameData& operator+=(const float time)
			{
				Time += time;
                return *this;
			}
		};
	public:
		void SetPerFrameTiming(const char* name, const float time)
		{
			std::scoped_lock<std::mutex> lock(m_PerFrameDataMutex);

			if (m_PerFrameData.find(name) == m_PerFrameData.end())
				m_PerFrameData[name] = 0.0f;

			PerFrameData& data = m_PerFrameData[name];
			data.Time += time;
			data.Samples++;
		}

		void Clear()
		{
			std::scoped_lock<std::mutex> lock(m_PerFrameDataMutex);
			m_PerFrameData.clear();
		}

		[[nodiscard]] const std::unordered_map<const char*, PerFrameData>& GetPerFrameData() const { return m_PerFrameData; }
	private:
		std::unordered_map<const char*, PerFrameData> m_PerFrameData;
		inline static std::mutex m_PerFrameDataMutex;
	};

	class ScopePerfTimer
	{
	public:
		ScopePerfTimer(const char* name, PerformanceProfiler* profiler)
			: m_Name(name), m_Profiler(profiler) {}

		~ScopePerfTimer()
		{
			float time = m_Timer.ElapsedMillis();
			m_Profiler->SetPerFrameTiming(m_Name, time);
		}
	private:
		const char* m_Name;
		PerformanceProfiler* m_Profiler;
		Timer m_Timer;
	};

#if 1
#define HZ_SCOPE_PERF(name)\
	ScopePerfTimer timer__LINE__(name, Application::Get().GetPerformanceProfiler());

#define HZ_SCOPE_TIMER(name)\
	ScopedTimer timer__LINE__(name);
#else
#define HZ_SCOPE_PERF(name)
#define HZ_SCOPE_TIMER(name)
#endif    
}
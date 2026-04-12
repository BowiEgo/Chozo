#pragma once

class FPSCounter {
public:
    void Update(float deltaTimeMillis) {
        m_AccumulatedTime += deltaTimeMillis;
        m_FrameCount++;

        if (deltaTimeMillis <= 0.0f && m_CurrentFPS > 0) return;

        // Update the average every 100ms for better readability
        if (m_AccumulatedTime >= 100.0f) {
            float seconds    = m_AccumulatedTime / 1000.0f;
            m_CurrentFPS     = static_cast<float>(m_FrameCount) / seconds;
            m_AverageLatency = m_AccumulatedTime / static_cast<float>(m_FrameCount);

            m_AccumulatedTime = 0.0f;
            m_FrameCount      = 0;
        }
    }

    float GetFPS() const { return m_CurrentFPS; }
    float GetAvgLatency() const { return m_AverageLatency; }

private:
    float m_AccumulatedTime = 0.0f;
    uint32_t m_FrameCount   = 0;
    float m_CurrentFPS      = 0.0f;
    float m_AverageLatency  = 0.0f;
};
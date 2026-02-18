#pragma once

class CORE_API FTimeStep {
public:
    FTimeStep(const float time = 0.0f) // NOLINT
        : m_Time(time) {}

    operator float() const { return m_Time; } // NOLINT

    float GetSeconds() const { return m_Time; }
    float GetMilliseconds() const { return m_Time * 1000.0f; }

private:
    float m_Time;
};
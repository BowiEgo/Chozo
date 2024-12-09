#pragma once

namespace Chozo {

    class TimeStep
    {
    public:
        TimeStep(const float time = 0.0f) // NOLINT
            : m_Time(time)
        {}

        operator float() const { return m_Time; } // NOLINT

        [[nodiscard]] float GetSeconds() const { return m_Time; }
        [[nodiscard]] float GetMilliseconds() const { return m_Time * 1000.0f; }
    private:
        float m_Time;
    };
}
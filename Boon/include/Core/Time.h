#pragma once
#include "Singleton.h"

#include <chrono>

namespace Boon
{
    using Clock = std::chrono::steady_clock;
    using TimePoint = std::chrono::time_point<Clock>;

    class Time final : public Singleton<Time>
    {
    public:
        void Start();
        void Step();
        void Wait();
        bool FixedStep();
        TimePoint Now();

        // Delta time is in seconds
        inline float GetDeltaTime() const { return m_DeltaTime; }
        inline float GetDuration() const { return std::chrono::duration<float>(m_CurrentTime - m_StartTime).count(); }
        inline const TimePoint& GetTime() const { return m_CurrentTime; }
        inline float GetFixedTimeStep() const { return m_FixedTimeStep; }
        inline void SetFixedTimeStep(float value) { m_FixedTimeStep = value; }

    private:
        float m_DeltaTime{};            // seconds
        float m_FixedTimeStep = 0.02f; // physics
        float m_InterpolationAlpha = 0.f;
        float m_Accumulator = 0.0f;

        TimePoint m_LastTime{};
        TimePoint m_CurrentTime{};
        TimePoint m_StartTime{};

        int m_MaxFPS{144};
    };
}

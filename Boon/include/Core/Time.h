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
        /**
         * @brief Start or reset the engine time tracking.
         *
         * Records the current time as the start time and resets internal
         * accumulators. Call once before running the main loop.
         */
        void Start();

        /**
         * @brief Advance time by computing delta since last step.
         *
         * Updates the internal delta time and accumulators. Should be called
         * once per frame.
         */
        void Step();

        /**
         * @brief Wait/sleep to enforce a maximum FPS if configured.
         *
         * May block the calling thread to maintain frame timing.
         */
        void Wait();

        /**
         * @brief Determine whether a fixed physics step should run.
         *
         * Uses the internal accumulator and fixed time step to indicate if
         * a physics update should be executed this frame.
         * @return True if a fixed step should be performed.
         */
        bool FixedStep();

        /**
         * @brief Get the current time point.
         * @return TimePoint representing the current steady clock time.
         */
        TimePoint Now();

        // Delta time is in seconds
        inline float GetDeltaTime() const { return m_DeltaTime; }
        inline float GetDuration() const { return std::chrono::duration<float>(m_CurrentTime - m_StartTime).count(); }
        inline const TimePoint& GetTime() const { return m_CurrentTime; }
        inline float GetFixedTimeStep() const { return m_FixedTimeStep; }
        inline void SetFixedTimeStep(float value) { m_FixedTimeStep = value; }
        inline float GetInterpolationAlpha() const { return m_InterpolationAlpha; }

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

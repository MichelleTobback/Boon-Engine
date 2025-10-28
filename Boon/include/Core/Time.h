#pragma once
#include "Singleton.h"

#include <chrono>

namespace Boon
{
	typedef std::chrono::steady_clock::time_point TimePoint;

	class Time final : public Singleton<Time>
	{
	public:
		void Start();
		void Step();
		void Wait();
		TimePoint Now();

		inline float GetDeltaTime() const { return m_DeltaTime / 1000.f; }
		inline float GetDuration() const { return std::chrono::duration<float>(m_CurrentTime - m_StartTime).count(); }
		inline const TimePoint& GetTime() const { return m_CurrentTime; }
		inline float GetFixedTimeStep() const { return m_FixedTimeStep; }
		inline void SetFixedTimeStep(float value) { m_FixedTimeStep = value; }

	private:
		float m_DeltaTime{};
		float m_FixedTimeStep{0.02f};
		TimePoint m_LastTime{};
		TimePoint m_CurrentTime{};
		TimePoint m_StartTime{};
		int m_MaxFPS{ 144 };
	};
}
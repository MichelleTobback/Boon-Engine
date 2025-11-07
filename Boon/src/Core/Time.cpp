#include "Core/Time.h"

#include <chrono>
#include <thread>

// Accumulator for fixed update

void Boon::Time::Start()
{
    m_StartTime = std::chrono::high_resolution_clock::now();
    m_LastTime = m_StartTime;
}

void Boon::Time::Step()
{
    m_CurrentTime = Now();
    m_DeltaTime = std::chrono::duration<float>(m_CurrentTime - m_LastTime).count(); // seconds
    m_LastTime = m_CurrentTime;

    m_Accumulator += m_DeltaTime;
}

bool Boon::Time::FixedStep()
{
    if (m_Accumulator >= m_FixedTimeStep)
    {
        m_Accumulator -= m_FixedTimeStep;
        return true;
    }

    m_InterpolationAlpha = m_Accumulator / m_FixedTimeStep;
    return false;
}

void Boon::Time::Wait()
{
    const int targetFrameTime = 1000 / m_MaxFPS;

	const auto frameEnd{ m_LastTime + std::chrono::milliseconds(targetFrameTime) };
	const auto sleepTime{ frameEnd - Now() };

	std::this_thread::sleep_for(sleepTime);
}

Boon::Clock::time_point Boon::Time::Now()
{
    return Clock::now();
}

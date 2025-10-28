#include "Core/Time.h"

#include <chrono>
#include <thread>

void Boon::Time::Start()
{
	m_StartTime = Now();
	m_LastTime = m_StartTime;
	return;
}

void Boon::Time::Step()
{
	m_CurrentTime = Now();

	m_DeltaTime = std::chrono::duration<float, std::milli>(m_CurrentTime - m_LastTime).count();

	m_LastTime = m_CurrentTime;
}

void Boon::Time::Wait()
{
	const int targetFrameTime = 1000 / m_MaxFPS;

	const auto frameEnd{ m_LastTime + std::chrono::milliseconds(targetFrameTime) };
	const auto sleepTime{ frameEnd - Now() };

	std::this_thread::sleep_for(sleepTime);
}

Boon::TimePoint Boon::Time::Now()
{
	return std::chrono::high_resolution_clock::now();
}

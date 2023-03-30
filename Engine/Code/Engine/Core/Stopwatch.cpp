#include "Clock.hpp"
#include "Stopwatch.hpp"
#include "Time.hpp"
#include "../Math/MathUtils.hpp"

Stopwatch::Stopwatch()
{
	m_clock = &g_systemClock;
	m_startTime = GetCurrentTimeSeconds();
	m_duration = 0.0;
}

Stopwatch::Stopwatch(double duration)
{
	m_clock = &g_systemClock;
	m_startTime = GetCurrentTimeSeconds();
	m_duration = duration;
}

Stopwatch::Stopwatch(const Clock* clock, double duration)
{
	m_clock = clock;
	m_startTime = GetCurrentTimeSeconds();
	m_duration = duration;
}

void Stopwatch::Start(double duration)
{
	if (m_clock == nullptr)
		m_clock = &g_systemClock;
	m_startTime = GetCurrentTimeSeconds();
	m_duration = duration;
}

void Stopwatch::Start(const Clock* clock, double duration)
{
	m_clock = clock;
	m_startTime = GetCurrentTimeSeconds();
	m_duration = duration;
}

void Stopwatch::Restart()
{
	m_startTime = GetCurrentTimeSeconds();
}

void Stopwatch::Stop()
{
	m_duration = 0.0;
}

void Stopwatch::Pause()
{
	m_isPaused = true;
	m_startTime -= GetCurrentTimeSeconds(); // to avoid fraction problem
}

void Stopwatch::Resume()
{
	m_isPaused = false;
}

double Stopwatch::GetElapsedTime() const
{
	// stopped
	if (m_duration == 0.0)
	{
		return 0.0;
	}

	// running or paused (negative)
	return GetCurrentTimeSeconds() - m_startTime; // default is to report elapsed time
}

float Stopwatch::GetElapsedFraction() const
{
	double elapsedTime = GetElapsedTime();
	while (elapsedTime >= m_duration)
	{
		elapsedTime -= m_duration;
	}
	return Clamp(static_cast<float>(elapsedTime / m_duration), 0.0f, 1.0f); // should always be [0, 1]
}

bool Stopwatch::IsStopped() const
{
	return (m_duration == 0) ? true : false;
}

bool Stopwatch::IsPaused() const
{
	return m_isPaused;
}

bool Stopwatch::HasDurationElapsed() const
{
	if (m_duration == 0.0)
	{
		return false;
	}

	if (GetElapsedTime() < m_duration)
	{
		return false;
	}
	return true; // matches decrement function calculation
}

bool Stopwatch::CheckDurationElapsedAndDecrement()
{
	// stopped
	if (m_duration == 0.0)
	{
		return false;
	}

	// less than one duration elapsed
	if (GetElapsedTime() < m_duration)
	{
		return false;
	}

	// more than one duration elapsed, decrement time by duration
	m_startTime += m_duration;
	return true;
}


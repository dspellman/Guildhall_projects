#include "Engine/Core/Job.hpp"
#include "Game\TestJob.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Math/Easing.hpp"

TestJob::TestJob(int id) 
	: Job(0xFFFF), m_id(id)
{
	m_waitTime = 50 + (int)(random.RollRandomIntInRange(0, 2950) * (0.5f + SmoothStart2(random.RollRandomFloatZeroToOne() * 0.5f)));
}

float TestJob::GetPercent()
{
	return (float)m_elapsed / (float)m_waitTime;
}

void TestJob::ForceComplete()
{
	m_elapsed = m_waitTime;
}

void TestJob::Execute()
{
	for (;;)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
		m_elapsed += 10;
		if (m_elapsed > m_waitTime)
		{
			return;
		}
	}
}

#include "Engine/Core/Clock.hpp"
#include "Time.hpp"

Clock g_systemClock;

Clock::Clock()
{
	m_parent = nullptr;
}

Clock::Clock(Clock& parent)
{
	m_parent = &parent;
	m_parent->AddChild(this);
}

Clock::~Clock()
{
	if (m_parent)
	{
		m_parent->RemoveChild(this);
	}

// 	for (Clock* child : m_children)
// 	{
// 		if (child)
// 		{
// 			delete child;
// 			child = nullptr;
// 		}
// 	}
}

void Clock::SetParent(Clock& parent)
{
	m_parent = &parent;
	m_parent->AddChild(this);
}

void Clock::Pause()
{
	m_isPaused = true;
}

void Clock::Unpause()
{
	m_isPaused = false;
	m_pauseAfterFrame = false;
}

void Clock::TogglePause()
{
	m_isPaused = !m_isPaused;
 	m_pauseAfterFrame = false;  // probably a good idea to unset
}

void Clock::StepFrame()
{
	m_isPaused = false;
	m_pauseAfterFrame = true;
}

void Clock::SetTimeDilation(double dilationAmount)
{
	m_timeDilation = dilationAmount;
}

double Clock::GetDeltaTime() const
{
	return m_deltaTime;
}

double Clock::GetTotalTime() const
{
	return m_totalTime;
}

size_t Clock::GetFrameCount() const
{
	return m_frameCount;
}

bool Clock::IsPaused() const
{
	return m_isPaused;
}

double Clock::GetTimeDilation() const
{
	return m_timeDilation;
}

void Clock::SystemBeginFrame()
{
	g_systemClock.Tick();
}

Clock& Clock::GetSystemClock()
{
	return g_systemClock;
}

void Clock::Tick()
{
	double timeNow = GetCurrentTimeSeconds();
	m_deltaTime = timeNow - m_lastUpdateTime;
	m_lastUpdateTime = timeNow;
	m_deltaTime = (m_deltaTime > 0.1) ? 0.1 : m_deltaTime; // clamp at maximum time
	Advance(m_deltaTime);
}

void Clock::Advance(double deltaTimeSeconds)
{
	m_deltaTime = m_isPaused ? 0.0 : deltaTimeSeconds;
	m_deltaTime *= m_timeDilation;
	m_totalTime += m_deltaTime;
	m_frameCount++;
	for (Clock* child : m_children)
	{
		if (child)
		{
			child->Advance(m_deltaTime);
		}
	}
	if (m_pauseAfterFrame)
	{
		m_isPaused = true;
		m_pauseAfterFrame = false;
	}

}

void Clock::Post()
{
	if (m_pauseAfterFrame)
	{
		m_isPaused = true;
		m_pauseAfterFrame = false; // reset every frame to single step one frame and wait
	}
}

void Clock::AddChild(Clock* childClock)
{
	for (int index = 0; index < static_cast<int>(m_children.size()); index++)
	{
		if (!m_children[index])
		{
			m_children[index] = childClock;
			return;
		}
	}
	m_children.push_back(childClock);
}

void Clock::RemoveChild(Clock* childClock)
{
	for (int index = 0; index < static_cast<int>(m_children.size()); index++)
	{
		if (m_children[index] == childClock)
		{
			m_children[index] = nullptr;
			// we could stop, but keep going for duplicates?
		}
	}
}

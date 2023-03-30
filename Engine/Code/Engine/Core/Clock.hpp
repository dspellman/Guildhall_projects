#pragma once
#include <vector>

class Clock
{
public:
	Clock();
	explicit Clock(Clock& parent);
	~Clock();
	Clock(const Clock& copy) = delete;

	void		SetParent(Clock& parent);

	void		Pause();
	void		Unpause();
	void		TogglePause();
	void		StepFrame();
	void		SetTimeDilation(double dilationAmount);

	double		GetDeltaTime() const;
	double		GetTotalTime() const;
	size_t		GetFrameCount() const;
	bool		IsPaused() const;
	double		GetTimeDilation() const;
	void		Post();

public:
	static void	SystemBeginFrame();
	static Clock& GetSystemClock();
	void		AddChild(Clock* childClock);
	void		RemoveChild(Clock* childClock);

protected:
	void		Tick();
	void		Advance(double deltaTimeSeconds);

	Clock* m_parent = nullptr;
protected:
	std::vector<Clock*>	m_children;

	double		m_lastUpdateTime = 0.0;
	double		m_totalTime = 0.0;
	double		m_deltaTime = 0.0;
	size_t		m_frameCount = 0;

	double		m_timeDilation = 1.0;
	bool		m_isPaused = false;
	bool		m_pauseAfterFrame = false;
};

extern Clock g_systemClock;
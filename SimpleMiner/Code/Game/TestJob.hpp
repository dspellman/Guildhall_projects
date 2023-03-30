#pragma once
#include <thread>
#include "Engine/Math/IntVec2.hpp"

class JobWorkerThread;

class TestJob : public Job
{
public:
	TestJob(int id);
	float GetPercent();
	void ForceComplete();

private:
	virtual void Execute() override;

public:
	int m_id;
	int m_waitTime = 1000;
	int m_elapsed = 0;
	std::thread m_thread;
};


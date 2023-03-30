#pragma once

enum class JobState
{
	UNKNOWN,
	QUEUED,
	PROCESSING,
	COMPLETE,
	ACTIVE,
	RETIRED,
};

class Job
{
public:
	friend class JobWorkerThread;
	virtual ~Job();
	Job(int jobType);
//private:
	virtual void Execute() = 0; // entry point to do the work of generating the chunk

public:
	JobState m_state = JobState::UNKNOWN;
	int m_jobType;
};

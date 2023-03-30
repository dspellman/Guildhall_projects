#pragma once
#include "Engine/Math/IntVec2.hpp"
#include <thread>
#include <atomic>
#include <mutex>
#include <deque>
#include "Job.hpp"
#include "JobWorkerThread.hpp"
#include <vector>

struct JobSystemConfig
{
	int m_workerThreads = 12;
};

class JobSystem
{
public:
	~JobSystem();
	JobSystem(JobSystemConfig& config);
	void Startup();
	void Shutdown();
	void BeginFrame();
	void EndFrame();

	void QueueJob(Job* job);
	Job* RetrieveJobToExecute(int jobType);
	void MoveToCompletedList(Job* job);
	Job* RetrieveCompletedJob(); // dynamic_cast<> to ChunkGenerateJob* to determine if it is

	std::deque<Job*> m_jobsQueue;
	std::mutex m_jobsQueueMutex;
	std::deque<Job*> m_jobsExecuting;
	std::mutex m_jobsExecutingMutex;
	std::deque<Job*> m_jobsCompleted;
	std::mutex m_jobsCompletedMutex;

	int m_workerThreads = 12;
	std::vector<JobWorkerThread*> m_threads = {};
	JobSystemConfig m_config = {};
};


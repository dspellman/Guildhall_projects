#pragma once
#include <thread>
#include <atomic>

class JobSystem;
class Job;

class JobWorkerThread
{
public:
	JobWorkerThread(int id, JobSystem* jobSystem, int jobType);
	static void JobWorkerMain(int threadID, JobWorkerThread* worker);
	virtual void Main();
	void join();

	int m_threadID = -1;
	int m_jobType = 0;
	std::thread m_thread;
	std::atomic<bool> m_isQuitting;
	JobSystem* m_jobSystem = nullptr;
};

#include "Engine/Core/JobWorkerThread.hpp"
#include "Engine/Core/JobSystem.hpp"
#include "Job.hpp"
#include <thread>
#include <chrono>

JobWorkerThread::JobWorkerThread(int id, JobSystem* jobSystem, int jobType)
	: m_threadID(id), m_jobSystem(jobSystem), m_jobType(jobType)
{
	m_thread = std::thread(&JobWorkerThread::JobWorkerMain, m_threadID, this);
}

void JobWorkerThread::JobWorkerMain(int threadID, JobWorkerThread* worker)
{
	if (threadID == -1)
	{
		return;
	}

	worker->Main();
}

void JobWorkerThread::Main()
{
	Job* job;
	while (!m_isQuitting)
	{
		job = m_jobSystem->RetrieveJobToExecute(m_jobType); // nominally a create job
		if (job)
		{
			job->Execute();
			m_jobSystem->MoveToCompletedList(job);
		}
		else
		{
			std::this_thread::sleep_for(std::chrono::microseconds(10));
		}
	}
}

void JobWorkerThread::join()
{
	m_thread.join();
}

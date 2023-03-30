#include "Engine/Core/JobSystem.hpp"
// #include "Game/ChunkGenerateJob.hpp"
// #include "Game/TestJob.hpp"
// #include "Game/Chunk.hpp"

// ChunkGenerateJob::ChunkGenerateJob(Chunk* chunk)
// 	: m_chunk(chunk)
// {
// 
// }

// void ChunkGenerateJob::Execute()
// {
// 	// generate the chunk data
// }

//--------------------------------------------------------------------
JobSystem::~JobSystem()
{
	for (auto thread : m_threads)
	{
		if (thread)
		{
			delete thread;
		}
	}
	m_threads.clear();
}

//--------------------------------------------------------------------
JobSystem::JobSystem(JobSystemConfig& config)
	: m_config(config)
{
	m_workerThreads = config.m_workerThreads;
}

//--------------------------------------------------------------------
void JobSystem::Startup()
{
	int cores = std::thread::hardware_concurrency();
	m_workerThreads = m_workerThreads > cores ? cores : m_workerThreads;

	for (int i = 0; i < m_workerThreads; i++)
	{
//		JobWorkerThread* thread = new JobWorkerThread(i, this, i ? JobType::JOB_CREATE : (JobType::JOB_LOAD | JobType::JOB_SAVE));
		JobWorkerThread* thread = new JobWorkerThread(i, this, i ? 1 : (2 | 4));
		m_threads.push_back(thread);
	}
}

//--------------------------------------------------------------------
void JobSystem::Shutdown()
{
	// this is not guaranteed to catch all jobs
	while(m_jobsQueue.size() || m_jobsExecuting.size())
	{
		std::this_thread::sleep_for(std::chrono::microseconds(100));
	}

	for (int index = 0; index < (int)m_threads.size(); index++)
	{
		JobWorkerThread* thread = m_threads[index];
		thread->m_isQuitting = true;
	}
	// join all threads before stopping
	for (int index = 0; index < (int)m_threads.size(); index++)
	{
		JobWorkerThread* thread = m_threads[index];
		thread->join();
	}
	for (int index = 0; index < (int)m_threads.size(); index++)
	{
		delete m_threads[index];
	}
	m_threads.clear();
}

//--------------------------------------------------------------------
void JobSystem::BeginFrame()
{

}

//--------------------------------------------------------------------
void JobSystem::EndFrame()
{

}

//--------------------------------------------------------------------
void JobSystem::QueueJob(Job* job)
{
	m_jobsQueueMutex.lock();
	job->m_state = JobState::QUEUED;
	m_jobsQueue.push_back(job);
	m_jobsQueueMutex.unlock();
}

//--------------------------------------------------------------------
Job* JobSystem::RetrieveJobToExecute(int jobTypes)
{
	Job *job = nullptr; // default to no job available
	// safely remove job from queue
	m_jobsQueueMutex.lock();
	for (auto index = m_jobsQueue.begin(); index < m_jobsQueue.end(); index++)
	{
		// use bit mask for jobs to return
		if ((*index)->m_jobType & jobTypes)
		{
			job = *index;
			m_jobsQueue.erase(index);
			break;
		}
	}
	m_jobsQueueMutex.unlock();

	if (job)
	{
		m_jobsExecutingMutex.lock();
		job->m_state = JobState::PROCESSING;
		m_jobsExecuting.push_back(job);
		m_jobsExecutingMutex.unlock();
	}

	return job;
}

//--------------------------------------------------------------------
void JobSystem::MoveToCompletedList(Job* job)
{
	if (job == nullptr)
	{
		return;
	}
	m_jobsExecutingMutex.lock();
	for (auto index = m_jobsExecuting.begin(); index < m_jobsExecuting.end(); index++)
	{
		if (*index == job)
		{
			m_jobsExecuting.erase(index);
			break;
		}
	}
	m_jobsExecutingMutex.unlock();

	m_jobsCompletedMutex.lock();
	job->m_state = JobState::COMPLETE;
	m_jobsCompleted.push_back(job);
	m_jobsCompletedMutex.unlock();
}

//--------------------------------------------------------------------
Job* JobSystem::RetrieveCompletedJob()
{
	Job* job = nullptr;
	// safely remove job from queue
	m_jobsCompletedMutex.lock();
	if (m_jobsCompleted.size())
	{
		job = m_jobsCompleted.front();
		m_jobsCompleted.pop_front();
		job->m_state = JobState::RETIRED;
	}
	m_jobsCompletedMutex.unlock();
	return job;
}

//--------------------------------------------------------------------


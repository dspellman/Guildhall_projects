#include "Engine/Core/Job.hpp"
#include "Game\ChunkGenerateJob.hpp"
#include "Game\Chunk.hpp"

ChunkGenerateJob::ChunkGenerateJob(Chunk* chunk) 
	: Job(JobType::JOB_CREATE), m_chunk(chunk)
{

}

void ChunkGenerateJob::Execute()
{
	if (m_chunk)
	{
		m_chunk->Create();
	}
}

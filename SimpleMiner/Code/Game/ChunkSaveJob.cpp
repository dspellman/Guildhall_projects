#include "Engine/Core/Job.hpp"
#include "Game\ChunkSaveJob.hpp"
#include "Game\Chunk.hpp"
#include "Game.hpp"

ChunkSaveJob::ChunkSaveJob(Chunk* chunk, int jobType) 
	: Job(jobType), m_chunk(chunk)
{

}

void ChunkSaveJob::Execute()
{
	if (m_chunk)
	{
		m_chunk->Deactivate();
	}
}

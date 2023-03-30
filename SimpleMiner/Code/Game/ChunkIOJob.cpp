#include "Engine/Core/Job.hpp"
#include "Game\ChunkIOJob.hpp"
#include "Game\Chunk.hpp"
#include "Game.hpp"

ChunkIOJob::ChunkIOJob(Chunk* chunk, int jobType) 
	: Job(jobType), m_chunk(chunk)
{

}

void ChunkIOJob::Execute()
{
	if (!m_chunk)
	{
		return; // this should be an error
	}
	if (m_jobType == JobType::JOB_LOAD)
	{
		char filename[80];
		sprintf_s(filename, "%s/Chunk(%i,%i).chunk", g_theGame->m_world->m_path.c_str(), m_chunk->m_chunkCoords.x, m_chunk->m_chunkCoords.y);
		m_chunk->ReadChunkFromDisc(filename);
	}
	else if (m_jobType == JobType::JOB_SAVE)
	{
		m_chunk->Deactivate();
	}
}

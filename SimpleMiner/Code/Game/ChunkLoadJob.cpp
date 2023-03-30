#include "Engine/Core/Job.hpp"
#include "Game\ChunkLoadJob.hpp"
#include "Game\Chunk.hpp"
#include "Game.hpp"

ChunkLoadJob::ChunkLoadJob(Chunk* chunk, int jobType) 
	: Job(jobType), m_chunk(chunk)
{

}

void ChunkLoadJob::Execute()
{
	if (m_chunk)
	{
		char filename[80];
		sprintf_s(filename, "%s/Chunk(%i,%i).chunk", g_theGame->m_world->m_path.c_str(), m_chunk->m_chunkCoords.x, m_chunk->m_chunkCoords.y);
		m_chunk->ReadChunkFromDisc(filename);
	}
}

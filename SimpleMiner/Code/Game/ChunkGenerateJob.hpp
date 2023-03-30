#pragma once
#include <thread>
#include "Engine/Math/IntVec2.hpp"

class Chunk;
class JobWorkerThread;

class ChunkGenerateJob : public Job
{
public:
	ChunkGenerateJob(Chunk* chunk);

private:
	virtual void Execute() override;

public:
//	IntVec2 m_chunkCoords;
	Chunk* m_chunk = nullptr;
	std::thread m_thread;
};


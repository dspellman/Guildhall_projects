#include "Game/World.hpp"
#include "Game/Game.hpp"
#include <direct.h>
#include "Game/BlockDefinition.hpp"
#include "Game/Entity.hpp"
#include <sys/stat.h>
#include "ChunkGenerateJob.hpp"
#include "ChunkLoadJob.hpp"
#include "ChunkSaveJob.hpp"

constexpr bool doMultithreaded = true;

World::~World()
{
	if (m_player)
	{
		delete m_player;
	}

	ClearChunkMap();
}

//------------------------------------------------------------------------------------
World::World(int worldSeed)
{
	m_chunkActivationRange = g_gameConfigBlackboard.GetValue("CHUNK_ACTIVATION_RANGE", CHUNK_ACTIVATION_RANGE);
	m_worldSeed = worldSeed;
	m_maxChunksRadiusX = 1 + int(m_chunkActivationRange) / SIZE_X;
	m_maxChunksRadiusY = 1 + int(m_chunkActivationRange) / SIZE_Y;
	m_inactiveRange = m_chunkActivationRange + SIZE_X + SIZE_Y;
	m_maxChunks = (2 * m_maxChunksRadiusX) * (2 * m_maxChunksRadiusY); 
	m_path = "Saves/";
	m_path += std::to_string(m_worldSeed);
	_mkdir(m_path.c_str()); // make sure we have this directory

	m_player = new Entity(KEYBOARD_XBOX);
	m_player->SetSizeAABB3(AABB3(Vec3::ZERO, Vec3(0.6f, 0.6f, 1.85f)), 1.65f);
}

//------------------------------------------------------------------------------------
bool fileExists(const char* filename)
{
	struct stat buf;
	if (stat(filename, &buf) != -1)
	{
		return true;
	}
	return false;
}

// convenience function to determine closest mesh to update
float World::CalcChunkToCameraDistance(Chunk* chunk)
{
	int x = chunk->m_chunkCoords.x;
	int y = chunk->m_chunkCoords.y;
	float distance = 0.0f;
	float nearestDistance = 9999.0;

	AABB2 bounds = Chunk::GetChunkWorldBounds(x, y);

	if ((distance = DistanceSquared(bounds.m_mins.x, bounds.m_mins.y, m_player->m_cameraPosition)) < nearestDistance * nearestDistance)
	{
		if (distance < nearestDistance)
		{
			nearestDistance = distance;
		}
	}
	if ((distance = DistanceSquared(bounds.m_mins.x, bounds.m_maxs.y, m_player->m_cameraPosition)) < nearestDistance * nearestDistance)
	{
		if (distance < nearestDistance)
		{
			nearestDistance = distance;
		}
	}
	if ((distance = DistanceSquared(bounds.m_maxs.x, bounds.m_mins.y, m_player->m_cameraPosition)) < nearestDistance * nearestDistance)
	{
		if (distance < nearestDistance)
		{
			nearestDistance = distance;
		}
	}
	if ((distance = DistanceSquared(bounds.m_maxs.x, bounds.m_maxs.y, m_player->m_cameraPosition)) < nearestDistance * nearestDistance)
	{
		if (distance < nearestDistance)
		{
			nearestDistance = distance;
		}
	}
	return nearestDistance;
}

//------------------------------------------------------------------------------------
void World::Update(float deltaSeconds)
{
	m_timeOfDay += (deltaSeconds * m_worldTimeScale) / (60.f * 60.f * 24.f);

	ChunkMap::iterator iter;
	// activate or deactivate chunks
	IntVec2 activateCandidate = IntVec2::ZERO;
	float activateCandidateRange = 2.0f * m_chunkActivationRange * m_chunkActivationRange;
	IntVec2 playerChunk = Chunk::GetChunkForWorldPosition(m_player->m_position);
	float distance = 0.0f;
	AABB2 bounds = AABB2::ZERO_TO_ONE;

	if (m_chunkCount < m_maxChunks)
	{
		for (int x = playerChunk.x - m_maxChunksRadiusX; x < playerChunk.x + m_maxChunksRadiusX; x++)
		{
			for (int y = playerChunk.y - m_maxChunksRadiusY; y < playerChunk.y + m_maxChunksRadiusY; y++)
			{
				bounds = Chunk::GetChunkWorldBounds(x, y);
				if ((distance = DistanceSquared(bounds.m_mins.x, bounds.m_mins.y, m_player->m_position)) < m_chunkActivationRange * m_chunkActivationRange)
				{
					if (distance < activateCandidateRange && GetMappedValue(IntVec2(x, y)) == nullptr && NotLiveChunk(IntVec2(x, y)))
					{
						activateCandidate = IntVec2(x, y);
						activateCandidateRange = distance;
					}
				}
				if ((distance = DistanceSquared(bounds.m_mins.x, bounds.m_maxs.y, m_player->m_position)) < m_chunkActivationRange * m_chunkActivationRange)
				{
					if (distance < activateCandidateRange && GetMappedValue(IntVec2(x, y)) == nullptr && NotLiveChunk(IntVec2(x, y)))
					{
						activateCandidate = IntVec2(x, y);
						activateCandidateRange = distance;
					}
				}
				if ((distance = DistanceSquared(bounds.m_maxs.x, bounds.m_mins.y, m_player->m_position)) < m_chunkActivationRange * m_chunkActivationRange)
				{
					if (distance < activateCandidateRange && GetMappedValue(IntVec2(x, y)) == nullptr && NotLiveChunk(IntVec2(x, y)))
					{
						activateCandidate = IntVec2(x, y);
						activateCandidateRange = distance;
					}
				}
				if ((distance = DistanceSquared(bounds.m_maxs.x, bounds.m_maxs.y, m_player->m_position)) < m_chunkActivationRange * m_chunkActivationRange)
				{
					if (distance < activateCandidateRange && GetMappedValue(IntVec2(x, y)) == nullptr && NotLiveChunk(IntVec2(x, y)))
					{
						activateCandidate = IntVec2(x, y);
						activateCandidateRange = distance;
					}
				}
			}
		}
	}

	// activate or deactivate one chunk if possible
	if (activateCandidateRange < 2.0f * m_chunkActivationRange * m_chunkActivationRange && m_chunkCount < m_maxChunks)
	{
		m_chunkCount++;
		Chunk* chunk = new Chunk();
		m_chunksLive[activateCandidate] = chunk;
		chunk->m_status = ChunkState::CHUNK_INITIALIZING;

		chunk->Initialize(activateCandidate);
		chunk->m_status = ChunkState::CHUNK_READY;

		char filename[80];
		sprintf_s(filename, "%s/Chunk(%i,%i).chunk", g_theGame->m_world->m_path.c_str(), activateCandidate.x, activateCandidate.y);
		if (fileExists(filename))
		{
			if (doMultithreaded)
			{
				Job* job = new ChunkLoadJob(chunk, JobType::JOB_LOAD); // assumes no failure
				chunk->m_status = ChunkState::CHUNK_QUEUED;
				g_theJobSystem->QueueJob(job);
			}
			else
			{
				chunk->ReadChunkFromDisc(filename);
				chunk->Activate(*this); // temporary
				m_chunks[activateCandidate] = chunk;
			}
		}
		else
		{
			if (doMultithreaded)
			{
				Job* job = new ChunkGenerateJob(chunk); // assumes no failure
				job->m_jobType = JobType::JOB_CREATE;
				chunk->m_status = ChunkState::CHUNK_QUEUED;
				g_theJobSystem->QueueJob(job);
			}
			else
			{
				// old version of the code
				chunk->Create();
				chunk->Activate(*this);
				m_chunks[activateCandidate] = chunk;
			}
		}
	}
	else if (m_chunkCount == m_maxChunks)
	{
		// test whether to deactivate
		AABB3 bounds3 = AABB3::ZERO_TO_ONE;
		IntVec2 deactivateCandidate = IntVec2::ZERO;
		float deactivateCandidateRange = 0.0f;
		distance = 0.0f;

		for (iter = m_chunks.begin(); iter != m_chunks.end(); iter++)
		{
			Chunk* chunk = iter->second;
			bounds3 = chunk->m_worldBounds;
			if ((distance = DistanceSquared(bounds3.GetCenter().x, bounds3.GetCenter().y, m_player->m_position)) > m_inactiveRange * m_inactiveRange)
			{
				if (distance > deactivateCandidateRange)
				{
					deactivateCandidate = chunk->m_chunkCoords;
					deactivateCandidateRange = distance;
				}
			}
		}

		if (deactivateCandidateRange > 0.0f)
		{
			m_chunkCount--;
			Chunk* chunk = GetMappedValue(deactivateCandidate);
			if (doMultithreaded)
			{
				m_chunks.erase(chunk->m_chunkCoords);
				m_chunksLive.erase(chunk->m_chunkCoords);
				UnlinkNeighbors(chunk);
				if (chunk->m_chunkCoords != deactivateCandidate)
				{
					ERROR_RECOVERABLE("Chunk coords corrupted");
				}
				Job* job = new ChunkSaveJob(chunk, JobType::JOB_SAVE); // assumes no failure
				chunk->m_status = ChunkState::CHUNK_QUEUED;
				g_theJobSystem->QueueJob(job);
			}
			else
			{
				UnlinkNeighbors(chunk);
				chunk->Deactivate();
				delete chunk;
				m_chunks.erase(deactivateCandidate);
				m_chunksLive.erase(deactivateCandidate); // put in the right place
			}
		}
	}

	// check completed jobs for chunks to activate
	Job* job = g_theJobSystem->RetrieveCompletedJob();
	if (job)
	{
		switch (job->m_jobType)
		{
		case JobType::JOB_CREATE:
			{
			ChunkGenerateJob* chunkJob = dynamic_cast<ChunkGenerateJob*>(job);
			Chunk* chunk1 = chunkJob->m_chunk;
			chunk1->Activate(*this);
			m_chunks[chunk1->m_chunkCoords] = chunk1;
			chunk1->m_status = ChunkState::CHUNK_ACTIVE;
			delete job;
			}
			break;
		case JobType::JOB_LOAD:
			{
			ChunkLoadJob* ioJob = dynamic_cast<ChunkLoadJob*>(job);
			Chunk* chunk2 = ioJob->m_chunk;
			chunk2->Activate(*this);
			m_chunks[chunk2->m_chunkCoords] = chunk2;
			chunk2->m_status = ChunkState::CHUNK_ACTIVE;
			delete job;
			}
			break;
		case JobType::JOB_SAVE:
			{
			ChunkSaveJob* ioJob = dynamic_cast<ChunkSaveJob*>(job);
			Chunk* chunk3 = ioJob->m_chunk;
//			UnlinkNeighbors(chunk3);
// 			m_chunks.erase(chunk3->m_chunkCoords);
// 			m_chunksLive.erase(chunk3->m_chunkCoords);
			delete chunk3;
			delete job;
			}
			break;
		default:
			delete job; // e.g. test job
			break;
		}
	}

	// do lighting update after activate/deactive and before updating chunks
	ProcessDirtyLighting();

	// Get closest two chunks that need mesh updates
	Chunk* nearest = nullptr;
	Chunk* nearby = nullptr;
	float nearestDistance = 9999.0f;
	float testDistance = 0.0f;

	for (iter = m_chunks.begin(); iter != m_chunks.end(); iter++)
	{
		Chunk* chunk = iter->second;
		if (chunk->m_needsMesh)
		{
			testDistance = CalcChunkToCameraDistance(chunk);
			if (testDistance < nearestDistance)
			{
				nearby = nearest;
				nearest = chunk;
				nearestDistance = testDistance;
			}
		}
	}
	if (nearest && nearest->m_needsMesh)
	{
		nearest->CreateGeometry();
		nearest->CreateBuffers();
		nearest->m_needsMesh = false;
	}
	if (nearby && nearby->m_needsMesh)
	{
		nearby->CreateGeometry();
		nearby->CreateBuffers();
		nearby->m_needsMesh = false;
	}

	// update chunks
	int meshCanUpdate = 2;
	for (iter = m_chunks.begin(); iter != m_chunks.end(); iter++)
	{
		Chunk* chunk = iter->second;
		if ((meshCanUpdate > 0) && chunk->m_needsMesh)
		{
			chunk->CreateGeometry();
			chunk->CreateBuffers();
			chunk->m_needsMesh = false;
			meshCanUpdate--;
		}
		chunk->Update(deltaSeconds);
	}

	// camera controls /////////////////////////////////////////
	if (g_theInput->WasKeyJustPressed('Y'))
	{
		m_worldTimeScale = 10000.0f;
	}
	if (g_theInput->WasKeyJustReleased('Y'))
	{
		m_worldTimeScale = 200.0f;
	}

	m_player->Update(deltaSeconds);
	m_player->UpdatePhysics(deltaSeconds);
	m_player->UpdateCameras();
}

//------------------------------------------------------------------------------------
void World::Render()
{
	std::vector<Vertex_PCU> vertexArray;

	// create basis at origin TEST DEBUG
	AddVertsForSphere(vertexArray, Vec3::ZERO, EulerAngles::ZERO, 0.06f);
	AddVertsForCylinder3D(vertexArray, Vec3::ZERO, Vec3(1.0f, 0.0f, 0.0f), 0.04f, Rgba8::RED);
	AddVertsForCylinder3D(vertexArray, Vec3::ZERO, Vec3(0.0f, 1.0f, 0.0f), 0.04f, Rgba8::GREEN);
	AddVertsForCylinder3D(vertexArray, Vec3::ZERO, Vec3(0.0f, 0.0f, 1.0f), 0.04f, Rgba8::BLUE);

	// create basis at camera 	AddVertsForCylinder3D(verts, position, orientation, length, radius, color, edges);
// 	Vec3 i, j, k;
// 	m_orientation.GetAsVectors_XFwd_YLeft_ZUp(i, j, k);
// 	Vec3 wcPos = m_position + 0.2f * i;
// 	AddVertsForCylinder3D(vertexArray, wcPos, wcPos + Vec3(0.01f, 0.0f, 0.0f), 0.0005f, Rgba8::RED);
// 	AddVertsForCylinder3D(vertexArray, wcPos, wcPos + Vec3(0.0f, 0.01f, 0.0f), 0.0005f, Rgba8::GREEN);
// 	AddVertsForCylinder3D(vertexArray, wcPos, wcPos + Vec3(0.0f, 0.0f, 0.01f), 0.0005f, Rgba8::BLUE);

	// draw hit normal if needed

	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->DrawVertexArray(int(vertexArray.size()), &vertexArray[0]);
	vertexArray.clear();

	ChunkMap::iterator iter;
	for (iter = m_chunks.begin(); iter != m_chunks.end(); iter++)
	{
		Chunk* chunk = iter->second;
		if (iter->first != chunk->m_chunkCoords)
		{
			ERROR_RECOVERABLE("Mismatch in render");
		}
		chunk->Render();
	}

	m_player->Render();
}

//------------------------------------------------------------------------------------
float World::DistanceSquared(float x, float y, Vec3 position)
{
	float xdiff = x - position.x;
	float ydiff = y - position.y;
	return xdiff * xdiff + ydiff * ydiff;
}

//------------------------------------------------------------------------------------
bool World::NotLiveChunk(IntVec2 const& keyName) const
{
	auto iter = m_chunksLive.find(keyName);
	if (iter == m_chunksLive.end())
	{
		return true;
	}
	return false;
}

//------------------------------------------------------------------------------------
bool World::EraseLiveChunk(IntVec2 const& keyName) const
{
	ChunkMap::const_iterator iter = m_chunks.find(keyName);
	if (iter == m_chunks.end())
	{
		return false;
	}
	return true;
}

//------------------------------------------------------------------------------------
Chunk* World::GetMappedValue(int x, int y) const
{
	return GetMappedValue(IntVec2(x, y));
}

//------------------------------------------------------------------------------------
Chunk* World::GetMappedValue(IntVec2 const& keyName) const
{
	ChunkMap::const_iterator iter = m_chunks.find(keyName);
	if (iter == m_chunks.end())
	{
		return nullptr;
	}
	return iter->second;
}

//------------------------------------------------------------------------------------
void World::ClearChunkMap()
{
	ChunkMap::iterator iter;
	for (iter = m_chunks.begin(); iter != m_chunks.end(); iter++)
	{
		Chunk* chunk = iter->second;
		chunk->Deactivate();
		delete chunk;
	}
	m_chunks.clear();
	m_chunkCount = 0;
}

//------------------------------------------------------------------------------------
void World::LinkNeighbors(Chunk* chunk)
{
	Chunk* neighbor = nullptr;
	neighbor = GetMappedValue(chunk->m_chunkCoords.x, chunk->m_chunkCoords.y + 1);
	if (neighbor)
	{
		neighbor->m_neighbors[SOUTH] = chunk;
		chunk->m_neighbors[NORTH] = neighbor;
		neighbor->m_needsMesh = true;
		chunk->m_needsMesh = true;
	}
	else
	{
		chunk->m_neighbors[NORTH] = nullptr;
		chunk->m_needsMesh = true;
	}

	neighbor = GetMappedValue(chunk->m_chunkCoords.x + 1, chunk->m_chunkCoords.y);
	if (neighbor)
	{
		neighbor->m_neighbors[WEST] = chunk;
		chunk->m_neighbors[EAST] = neighbor;
		neighbor->m_needsMesh = true;
		chunk->m_needsMesh = true;
	}
	else
	{
		chunk->m_neighbors[EAST] = nullptr;
		chunk->m_needsMesh = true;
	}

	neighbor = GetMappedValue(chunk->m_chunkCoords.x, chunk->m_chunkCoords.y - 1);
	if (neighbor)
	{
		neighbor->m_neighbors[NORTH] = chunk;
		chunk->m_neighbors[SOUTH] = neighbor;
		neighbor->m_needsMesh = true;
		chunk->m_needsMesh = true;
	}
	else
	{
		chunk->m_neighbors[SOUTH] = nullptr;
		chunk->m_needsMesh = true;
	}

	neighbor = GetMappedValue(chunk->m_chunkCoords.x - 1, chunk->m_chunkCoords.y);
	if (neighbor)
	{
		neighbor->m_neighbors[EAST] = chunk;
		chunk->m_neighbors[WEST] = neighbor;
		neighbor->m_needsMesh = true;
		chunk->m_needsMesh = true;
	}
	else
	{
		chunk->m_neighbors[WEST] = nullptr;
		chunk->m_needsMesh = true;
	}
}

//------------------------------------------------------------------------------------
void World::UnlinkNeighbors(Chunk* chunk)
{
	Chunk* neighbor = nullptr;
	neighbor = GetMappedValue(chunk->m_chunkCoords.x, chunk->m_chunkCoords.y + 1);
	if (neighbor)
	{
		neighbor->m_neighbors[SOUTH] = nullptr;
		neighbor->m_needsMesh = true;
	}

	neighbor = GetMappedValue(chunk->m_chunkCoords.x + 1, chunk->m_chunkCoords.y);
	if (neighbor)
	{
		neighbor->m_neighbors[WEST] = nullptr;
		neighbor->m_needsMesh = true;
	}

	neighbor = GetMappedValue(chunk->m_chunkCoords.x, chunk->m_chunkCoords.y - 1);
	if (neighbor)
	{
		neighbor->m_neighbors[NORTH] = nullptr;
		neighbor->m_needsMesh = true;
	}

	neighbor = GetMappedValue(chunk->m_chunkCoords.x - 1, chunk->m_chunkCoords.y);
	if (neighbor)
	{
		neighbor->m_neighbors[EAST] = nullptr;
		neighbor->m_needsMesh = true;
	}
}

//------------------------------------------------------------------------------------
void World::ChangeLightToAtLeastOneLessThanNeighbor(uint8_t& indoor, uint8_t& outdoor, BlockIterator neighbor)
{
	if (neighbor.GetBlock() == nullptr)
	{
		return;
	}
	if (neighbor.GetBlock()->GetIndoorLight() > indoor)
	{
		indoor = neighbor.GetBlock()->GetIndoorLight() - 1;
	}
	if (neighbor.GetBlock()->GetOutdoorLight() > outdoor)
	{
		outdoor = neighbor.GetBlock()->GetOutdoorLight() - 1;
	}
}

//------------------------------------------------------------------------------------
void World::ProcessDirtyLighting()
{
	while (!m_queue.empty())
	{
		ProcessNextDirtyLightBlock(m_queue.front());
		m_queue.pop_front();
	}
}

//------------------------------------------------------------------------------------
void World::ProcessNextDirtyLightBlock(BlockIterator& blockIterator)
{
	blockIterator.GetBlock()->SetLightDirty(false);
	// calculate theoretical correct lighting for block
	uint8_t indoor = blockIterator.GetBlock()->GetLightEmitted();
	uint8_t outdoor = 0;
	if (blockIterator.GetBlock()->IsSky())
	{
		outdoor = MAX_LIGHT;
	}

	// add outdoor glow lighting later if ever
	// test indoor light of neighbors
	if (blockIterator.GetBlock()->IsOpaque() == false)
	{
		ChangeLightToAtLeastOneLessThanNeighbor(indoor, outdoor, blockIterator.GetEastNeighbor());
		ChangeLightToAtLeastOneLessThanNeighbor(indoor, outdoor, blockIterator.GetNorthNeighbor());
		ChangeLightToAtLeastOneLessThanNeighbor(indoor, outdoor, blockIterator.GetWestNeighbor());
		ChangeLightToAtLeastOneLessThanNeighbor(indoor, outdoor, blockIterator.GetSouthNeighbor());
		ChangeLightToAtLeastOneLessThanNeighbor(indoor, outdoor, blockIterator.GetTopNeighbor());
		ChangeLightToAtLeastOneLessThanNeighbor(indoor, outdoor, blockIterator.GetBottomNeighbor());
	}

	// mark chunk meshes in need of updating if lighting is incorrect
	if (blockIterator.GetBlock()->GetIndoorLight() != indoor || blockIterator.GetBlock()->GetOutdoorLight() != outdoor)
	{
		blockIterator.GetBlock()->SetIndoorLight(indoor);
		blockIterator.GetBlock()->SetOutdoorLight(outdoor);
		blockIterator.m_chunk->m_needsMesh = true;
		MarkNeighborsLightingDirty(blockIterator);
	}
}

//------------------------------------------------------------------------------------
void World::MarkLightDirtyIfNotOpaque(BlockIterator blockIterator)
{
	if (blockIterator.m_chunk == nullptr)
	{
		return;
	}
	blockIterator.m_chunk->m_needsMesh = true;
	if (blockIterator.GetBlock()->IsOpaque() == false)
	{
		MarkLightingDirty(blockIterator);
	}
}

//------------------------------------------------------------------------------------
void World::MarkNeighborsLightingDirty(BlockIterator blockIterator)
{
	MarkLightDirtyIfNotOpaque(blockIterator.GetEastNeighbor());
	MarkLightDirtyIfNotOpaque(blockIterator.GetNorthNeighbor());
	MarkLightDirtyIfNotOpaque(blockIterator.GetWestNeighbor());
	MarkLightDirtyIfNotOpaque(blockIterator.GetSouthNeighbor());
	MarkLightDirtyIfNotOpaque(blockIterator.GetTopNeighbor());
	MarkLightDirtyIfNotOpaque(blockIterator.GetBottomNeighbor());
}

//------------------------------------------------------------------------------------
void World::MarkLightingDirty(BlockIterator blockIterator)
{
	if (blockIterator.GetBlock()->IsLightDirty())
	{
		return;
	}
	blockIterator.GetBlock()->SetLightDirty(true);
	m_queue.push_back(blockIterator);
}

//------------------------------------------------------------------------------------
void World::MarkLightingDirty(Chunk* chunk, int index)
{
	if (chunk->m_block[index].IsLightDirty())
	{
		return;
	}
	chunk->m_block[index].SetLightDirty(true);
	m_queue.push_back(BlockIterator(chunk, index));
}

//------------------------------------------------------------------------------------
void World::UndirtyAllBlocksInChunk(Chunk* chunk)
{
	std::deque<BlockIterator>::iterator qIterator =	m_queue.begin();
	while (qIterator != m_queue.end())
	{
		if (qIterator->m_chunk == chunk)
		{
			chunk->m_block[qIterator->m_blockIndex].SetLightDirty(false);
			m_queue.erase(qIterator);
		}
		qIterator++;
	}
}

//------------------------------------------------------------------------------------
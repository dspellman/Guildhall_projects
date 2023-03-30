#pragma once
#include "Game/GameCommon.hpp"
#include "Game/Chunk.hpp"
#include <vector>
#include <deque>
#include "BlockIterator.hpp"
#include <set>

typedef std::map< IntVec2, Chunk* > ChunkMap;
class Entity;

class World
{
public:
	virtual ~World();
	World(int worldSeed);
	float CalcChunkToCameraDistance(Chunk* chunk);
	void Update(float deltaSeconds);
	void Render();

	float DistanceSquared(float x, float y, Vec3 position);
	bool NotLiveChunk(IntVec2 const& keyName) const;
	bool EraseLiveChunk(IntVec2 const& keyName) const;
	Chunk* GetMappedValue(IntVec2 const& keyName) const;
	Chunk* GetMappedValue(int x, int y) const;
	void ClearChunkMap();
	void LinkNeighbors(Chunk* chunk);
	void UnlinkNeighbors(Chunk* chunk);
	void ChangeLightToAtLeastOneLessThanNeighbor( uint8_t& indoor, uint8_t& outdoor, BlockIterator neighbor );
	void ProcessDirtyLighting();
	void ProcessNextDirtyLightBlock(BlockIterator& blockIterator);
	void MarkLightDirtyIfNotOpaque(BlockIterator blockIterator);
	void MarkNeighborsLightingDirty(BlockIterator blockIterator);
	void MarkLightingDirty(BlockIterator blockIterator);
	void MarkLightingDirty(Chunk* chunk, int index);
	void UndirtyAllBlocksInChunk(Chunk* chunk);

	ChunkMap m_chunks;
	ChunkMap m_chunksLive;
	std::string m_path;

	int m_worldSeed = 0;
	float m_worldTimeScale = 200.0f;
	float m_timeOfDay = 0.5f;
	float m_chunkActivationRange = CHUNK_ACTIVATION_RANGE;
	float m_inactiveRange = 0.0f;
	int m_maxChunksRadiusX = 0;
	int m_maxChunksRadiusY = 0;
	int m_maxChunks = 0;
	int m_chunkCount = 0;
	std::deque<BlockIterator> m_queue;

	Entity* m_player;
};
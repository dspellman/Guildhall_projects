#pragma once
#include "Game/GameCommon.hpp"

class Chunk;
class Block;

class BlockIterator
{
public:
	BlockIterator();
	BlockIterator(Chunk* chunk, int index);
	BlockIterator(Vec3 position);

	Chunk* m_chunk = nullptr;
	int m_blockIndex = 0;
	int m_x = 0;
	int m_y = 0;
	int m_z = 0;

	Block* GetBlock();
	Vec3 GetWorldCenter();
	int GetIndex(int x, int y, int z);
	BlockIterator GetEastNeighbor();
	BlockIterator GetNorthNeighbor();
	BlockIterator GetWestNeighbor();
	BlockIterator GetSouthNeighbor();
	BlockIterator GetTopNeighbor();
	BlockIterator GetBottomNeighbor();
	int IntCoord(float point) const;
	bool IsTileSolid(BlockIterator blockIterator) const;
	struct RaycastHit RaycastVsBlocksFlawless(Vec3 startPosition, Vec3 forwardNormal, float maxDist) const;
};

struct RaycastHit
{
	bool m_hit = false;
	bool m_self = false;
	Vec3 m_start = Vec3::ZERO;
	Vec3 m_hitPoint = Vec3::ZERO;
	float m_distance = 0.0f;
	Vec3 m_impactSurfaceNormal = Vec3::ZERO;
	IntVec3 m_blockCoords = IntVec3::ZERO;
	BlockIterator blockIterator;
};

#include "Game\BlockIterator.hpp"
#include "Chunk.hpp"
#include "Game/Game.hpp"

BlockIterator::BlockIterator()
{
	m_chunk = nullptr;
	m_x = 0;
	m_y = 0;
	m_z = 0;
	m_blockIndex = 0;
}

BlockIterator::BlockIterator(Chunk* chunk, int index)
	: m_chunk(chunk), m_blockIndex(index)
{
	m_x = index & MASK_X;
	m_y = (index >> BITS_X) & MASK_Y;
	m_z = (index >> (BITS_X + BITS_Y)) & MASK_Z;
}

BlockIterator::BlockIterator(Vec3 position)
{
	m_chunk = g_theGame->m_world->GetMappedValue(Chunk::GetChunkForWorldPosition(position));
	m_blockIndex = Chunk::GetBlockForPosition(position);
	m_x = m_blockIndex & MASK_X;
	m_y = (m_blockIndex >> BITS_X) & MASK_Y;
	m_z = (m_blockIndex >> (BITS_X + BITS_Y)) & MASK_Z;
}

Block* BlockIterator::GetBlock()
{
	if (m_chunk == nullptr)
	{
		return nullptr;
	}
	return m_chunk->GetBlockPtr(m_blockIndex);
}

Vec3 BlockIterator::GetWorldCenter()
{
	if (m_chunk == nullptr)
	{
		return Vec3::ZERO;
	}
	AABB3 bounds = m_chunk->GetBlockBounds(m_blockIndex);
	return bounds.GetCenter();
}

int BlockIterator::GetIndex(int x, int y, int z)
{
	return z << (BITS_X + BITS_Y) | y << BITS_X | x;
}

// get neighboring blocks functions
BlockIterator BlockIterator::GetEastNeighbor()
{
	if (!m_chunk)
	{
		return BlockIterator(nullptr, GetIndex(0, m_y, m_z));
	}
	if (m_x == MASK_X)
	{
		return BlockIterator(m_chunk->m_neighbors[EAST], GetIndex(0, m_y, m_z));
	}
	else
	{
		return BlockIterator(m_chunk, GetIndex(m_x + 1, m_y, m_z));
	}
}

BlockIterator BlockIterator::GetNorthNeighbor()
{
	if (!m_chunk)
	{
		return BlockIterator(nullptr, GetIndex(m_x, 0, m_z));
	}
	if (m_y == MASK_Y)
	{
		return BlockIterator(m_chunk->m_neighbors[NORTH], GetIndex(m_x, 0, m_z));
	}
	else
	{
		return BlockIterator(m_chunk, GetIndex(m_x, m_y + 1, m_z));
	}
}

BlockIterator BlockIterator::GetWestNeighbor()
{
	if (!m_chunk)
	{
		return BlockIterator(nullptr, GetIndex(MASK_X, m_y, m_z));
	}
	if (m_x == 0)
	{
		return BlockIterator(m_chunk->m_neighbors[WEST], GetIndex(MASK_X, m_y, m_z));
	}
	else
	{
		return BlockIterator(m_chunk, GetIndex(m_x - 1, m_y, m_z));
	}
}

BlockIterator BlockIterator::GetSouthNeighbor()
{
	if (!m_chunk)
	{
		return BlockIterator(nullptr, GetIndex(m_x, MASK_Y, m_z));
	}
	if (m_y == 0)
	{
		return BlockIterator(m_chunk->m_neighbors[SOUTH], GetIndex(m_x, MASK_Y, m_z));
	}
	else
	{
		return BlockIterator(m_chunk, GetIndex(m_x, m_y - 1, m_z));
	}
}

BlockIterator BlockIterator::GetTopNeighbor()
{
	if (m_z == MASK_Z)
	{
		return BlockIterator(nullptr, GetIndex(m_x, m_y, m_z + 1));
	}
	else
	{
		return BlockIterator(m_chunk, GetIndex(m_x, m_y, m_z + 1));
	}
}

BlockIterator BlockIterator::GetBottomNeighbor()
{
	if (m_z == 0)
	{
		return BlockIterator(nullptr, GetIndex(m_x, m_y, m_z - 1));
	}
	else
	{
		return BlockIterator(m_chunk, GetIndex(m_x, m_y, m_z - 1));
	}
}

int BlockIterator::IntCoord(float point) const
{
	return RoundDownToInt(point);
}

bool BlockIterator::IsTileSolid(BlockIterator blockIterator) const
{
// 	if (m_chunk && m_chunk->GetBlockPtr(m_blockIndex)->IsSolid())
	if (blockIterator.GetBlock() && blockIterator.GetBlock()->IsSolid())
	{
		return true;
	}
	return false;
}

struct RaycastHit BlockIterator::RaycastVsBlocksFlawless(Vec3 startPosition, Vec3 forwardNormal, float maxDist) const
{
	struct RaycastHit hit;

	int tileX = IntCoord(startPosition.x);
	int tileY = IntCoord(startPosition.y);
	int tileZ = IntCoord(startPosition.z);
	float fwdDistPerXCrossing = 1.0f / fabsf(forwardNormal.x);
	float fwdDistPerYCrossing = 1.0f / fabsf(forwardNormal.y);
	float fwdDistPerZCrossing = 1.0f / fabsf(forwardNormal.z);
	int tileStepDirectionX = forwardNormal.x < 0.0f ? -1 : 1;
	int tileStepDirectionY = forwardNormal.y < 0.0f ? -1 : 1;
	int tileStepDirectionZ = forwardNormal.z < 0.0f ? -1 : 1;

	float xAtFirstXCrossing = static_cast<float>(tileX) + static_cast<float>(tileStepDirectionX + 1) / 2.0f;
	float xDistToFirstXCrossing = xAtFirstXCrossing - startPosition.x;
	float fwdDistAtNextXCrossing = fabsf(xDistToFirstXCrossing) * fwdDistPerXCrossing;

	float yAtFirstYCrossing = static_cast<float>(tileY) + static_cast<float>(tileStepDirectionY + 1) / 2.0f;
	float yDistToFirstYCrossing = yAtFirstYCrossing - startPosition.y;
	float fwdDistAtNextYCrossing = fabsf(yDistToFirstYCrossing) * fwdDistPerYCrossing;

	float zAtFirstZCrossing = static_cast<float>(tileZ) + static_cast<float>(tileStepDirectionZ + 1) / 2.0f;
	float zDistToFirstZCrossing = zAtFirstZCrossing - startPosition.z;
	float fwdDistAtNextZCrossing = fabsf(zDistToFirstZCrossing) * fwdDistPerZCrossing;

// 	BlockIterator blockIterator(startPosition);
	BlockIterator blockIterator = *this;

	// if starting tile is solid, return immediate hit
	if (IsTileSolid(blockIterator))
	{
		hit.m_distance = 0.0f;
		hit.m_hit = true;
		hit.m_self = true; // flag that we hit our starting block
		hit.m_start = startPosition;
		hit.m_hitPoint = startPosition;
		// determine impact surface normal according to what it would be if we followed the trajectory to the next tile
		if (fwdDistAtNextZCrossing < fwdDistAtNextXCrossing && fwdDistAtNextZCrossing < fwdDistAtNextYCrossing)
		{
			hit.m_impactSurfaceNormal = Vec3(0.0f, 0.0f, static_cast<float>(-tileStepDirectionZ));
		}
		else
		{
			hit.m_impactSurfaceNormal = (fwdDistAtNextXCrossing < fwdDistAtNextYCrossing) ? Vec3(static_cast<float>(-tileStepDirectionX), 0.0f, 0.0f) : Vec3(0.0f, static_cast<float>(-tileStepDirectionY), 0.0f);
		}
		hit.m_blockCoords = IntVec3(tileX, tileY, tileZ);
		hit.blockIterator = blockIterator;
		return hit;
	}

	for (;;)
	{
		if (fwdDistAtNextZCrossing < fwdDistAtNextXCrossing && fwdDistAtNextZCrossing < fwdDistAtNextYCrossing)
		{
			if (fwdDistAtNextZCrossing > maxDist)
			{
				hit.m_distance = maxDist;
				hit.m_hit = false;
				hit.m_self = false;
				hit.m_start = startPosition;
				hit.m_hitPoint = startPosition + forwardNormal * maxDist;
				hit.m_impactSurfaceNormal = Vec3::ZERO;
				hit.m_blockCoords = IntVec3(tileX, tileY, tileZ); // this seems wrong if it is used later?
				hit.blockIterator = blockIterator;
				return hit; // no hit
			}
			tileZ += tileStepDirectionZ;
			blockIterator = tileStepDirectionZ < 0 ? blockIterator.GetBottomNeighbor() : blockIterator.GetTopNeighbor();
			if (IsTileSolid(blockIterator))
			{
				hit.m_distance = fwdDistAtNextZCrossing;
				hit.m_hit = true;
				hit.m_self = false;
				hit.m_start = startPosition;
				hit.m_hitPoint = startPosition + forwardNormal * fwdDistAtNextZCrossing;
				hit.m_impactSurfaceNormal = Vec3(0.0f, 0.0f, static_cast<float>(-tileStepDirectionZ));
				hit.m_blockCoords = IntVec3(tileX, tileY, tileZ);
				hit.blockIterator = blockIterator;
				return hit; // hit
			}
			fwdDistAtNextZCrossing += fwdDistPerZCrossing;
		}
		else
		{
			if (fwdDistAtNextXCrossing < fwdDistAtNextYCrossing)
			{
				if (fwdDistAtNextXCrossing > maxDist)
				{
					hit.m_distance = maxDist;
					hit.m_hit = false;
					hit.m_self = false;
					hit.m_start = startPosition;
					hit.m_hitPoint = startPosition + forwardNormal * maxDist;
					hit.m_impactSurfaceNormal = Vec3::ZERO;
					hit.m_blockCoords = IntVec3(tileX, tileY, tileZ);
					hit.blockIterator = blockIterator;
					return hit; // no hit
				}
				tileX += tileStepDirectionX;
				blockIterator = tileStepDirectionX < 0 ? blockIterator.GetWestNeighbor() : blockIterator.GetEastNeighbor();
				if (IsTileSolid(blockIterator))
				{
					hit.m_distance = fwdDistAtNextXCrossing;
					hit.m_hit = true;
					hit.m_self = false;
					hit.m_start = startPosition;
					hit.m_hitPoint = startPosition + forwardNormal * fwdDistAtNextXCrossing;
					hit.m_impactSurfaceNormal = Vec3(static_cast<float>(-tileStepDirectionX), 0.0f, 0.0f);
					hit.m_blockCoords = IntVec3(tileX, tileY, tileZ);
					hit.blockIterator = blockIterator;
					return hit; // hit
				}
				fwdDistAtNextXCrossing += fwdDistPerXCrossing;
			}
			else
			{
				if (fwdDistAtNextYCrossing > maxDist)
				{
					hit.m_distance = maxDist;
					hit.m_hit = false;
					hit.m_self = false;
					hit.m_start = startPosition;
					hit.m_hitPoint = startPosition + forwardNormal * maxDist;
					hit.m_impactSurfaceNormal = Vec3::ZERO;
					hit.m_blockCoords = IntVec3(tileX, tileY, tileZ);
					hit.blockIterator = blockIterator;
					return hit; // no hit
				}
				tileY += tileStepDirectionY;
				blockIterator = tileStepDirectionY < 0 ? blockIterator.GetSouthNeighbor() : blockIterator.GetNorthNeighbor();
				if (IsTileSolid(blockIterator))
				{
					hit.m_distance = fwdDistAtNextYCrossing;
					hit.m_hit = true;
					hit.m_self = false;
					hit.m_start = startPosition;
					hit.m_hitPoint = startPosition + forwardNormal * fwdDistAtNextYCrossing;
					hit.m_impactSurfaceNormal = Vec3(0.0f, static_cast<float>(-tileStepDirectionY), 0.0f);
					hit.m_blockCoords = IntVec3(tileX, tileY, tileZ);
					hit.blockIterator = blockIterator;
					return hit; // hit
				}
				fwdDistAtNextYCrossing += fwdDistPerYCrossing;
			}
		}
	}
}

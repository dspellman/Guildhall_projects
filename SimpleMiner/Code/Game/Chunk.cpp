#include "Game/Chunk.hpp"
#include <vector>
#include <stdint.h>
#include <string>
#include "Engine/Core/FileUtils.hpp"
#include "Engine/Math/Easing.hpp"
#include "ThirdParty/Squirrel/SmoothNoise.hpp"
#include <corecrt_math.h>
#include "Game/World.hpp"
#include "Game/Game.hpp"
#include "BlockTemplate.hpp"
#include "Engine/Renderer/DebugRenderMode.hpp"
#include "BuildingTemplate.hpp"
#include "ThirdParty/Squirrel/RawNoise.hpp"

bool indexedDraw = true; // TEST DEBUG

Chunk::~Chunk()
{
	if (m_block)
	{
		delete[] m_block;
	}

	if (m_immediateVBO_PCU)
	{
		delete m_immediateVBO_PCU;
		m_immediateVBO_PCU = nullptr;
	}

	if (m_indexBuffer)
	{
		delete m_indexBuffer;
		m_indexBuffer = nullptr;
	}
}

Chunk::Chunk()
{
	m_vertexes.reserve(BLOCKSPERCHUNK * 4 * 4);
	m_indexes.reserve(BLOCKSPERCHUNK * 6 * 4);
	m_block = new Block[BLOCKSPERCHUNK];
	m_worldSeed = g_gameConfigBlackboard.GetValue("WORLD_SEED", WORLD_SEED);
}

//--------------------------------------------------------------------------------
bool Chunk::Create()
{
	m_status = ChunkState::CHUNK_GENERATING;
	// generate world coords of block 0
	int baseX = m_chunkCoords.x << BITS_X;
	int baseY = m_chunkCoords.y << BITS_X;

	// generate arrays of Perlin noise for each type we need
	for (int y = 0; y < NOISE_DIM; y++)
	{
		for (int x = 0; x < NOISE_DIM; x++)
		{
			int index = y * NOISE_DIM + x;
			float dx = float(baseX + x - TREE_DIAMETER);
			float dy = float(baseY + y - TREE_DIAMETER);
			// need global index values - buffer as floats
			m_humidity[index] = 0.5f + 0.5f * Compute2dPerlinNoise( dx, dy, HUMIDITY_SCALE, 6, 0.5f, 2.0f, true, m_worldSeed + 1 );
			m_temperature[index] = 0.5f + 0.5f * Compute2dPerlinNoise( dx, dy, TEMP_SCALE, 8, 0.6f, 2.0f, true, m_worldSeed + 2 );
			m_hilliness[index] = 0.5f + 0.5f * Compute2dPerlinNoise( dx, dy, HILL_SCALE, 5, 0.6f, 2.0f, true, m_worldSeed + 3 );
			m_ocean[index] = Compute2dPerlinNoise( dx, dy, OCEAN_SCALE, 5, 0.5f, 2.0f, true, m_worldSeed + 4 );
			m_treeDensity[index] = 0.5f + 0.5f * Compute2dPerlinNoise(dx, dy, 200.0f, 10, 0.6f, 2.0f, true, m_worldSeed + 6);
			m_forest[index] = 0.5f + 0.5f * Compute2dPerlinNoise( dx, dy, 400.0f, 9, m_treeDensity[index], 2.0f, true, m_worldSeed + 5 );
		}
	}

	for (int y = 0; y < NOISE_DIM; y++)
	{
		for (int x = 0; x < NOISE_DIM; x++)
		{
			int index = y * NOISE_DIM + x;
			float dx = float(m_chunkCoords.x + x - ((VILLAGE_RANGE + VILLAGE_DIAMETER) >> 1));
			float dy = float(m_chunkCoords.y + y - ((VILLAGE_RANGE + VILLAGE_DIAMETER) >> 1));
			m_town[index] = 0.5f + 0.5f * Compute2dPerlinNoise(dx, dy, 200.0f, 9, 0.5f, 2.0f, true, m_worldSeed + 7);
		}
	}

	// create terrain
	for (int y = 0; y < SIZE_Y; y++)
	{
		for (int x = 0; x < SIZE_X; x++)
		{
			float tx = float(baseX + x);
			float ty = float(baseY + y);
			int offset = (y + TREE_DIAMETER) * NOISE_DIM + (x + TREE_DIAMETER);
			int terrainHeight = 0;

			uint8_t block = DetermineSurfaceTerrain(offset, tx, ty, &terrainHeight);
			SetBlock(x, y, terrainHeight, block);

			// create terrain below surface block
			float humid = m_humidity[offset];
			for (int index = random.RollRandomIntInRange(3, 4); index > 0; index--)
			{
				terrainHeight--; // move down to next block to create
				if (index > Interpolate(1, 10, humid))
				{
					SetBlock(x, y, terrainHeight, SAND);
				}
				else
				{
					SetBlock(x, y, terrainHeight, DIRT);
				}
			}

			// create filler blocks for rest of column
			while (terrainHeight > 0)
			{
				terrainHeight--; // move down to next block to create
				float type = random.RollRandomFloatZeroToOne();
				if (type < 0.001)
				{
					SetBlock(x, y, terrainHeight, DIAMOND);
				}
				else if (type < 0.003)
				{
					SetBlock(x, y, terrainHeight, GLOWSTONE);
				}
				else if (type < 0.005)
				{
					SetBlock(x, y, terrainHeight, GOLD);
				}
				else if(type < 0.02)
				{
					SetBlock(x, y, terrainHeight, IRON);
				}
				else if(type < 0.05)
				{
					SetBlock(x, y, terrainHeight, COAL);
				}
				else
				{
					SetBlock(x, y, terrainHeight, STONE);
				}
			}

			// it would be better to count down from the top and only make water if air is above the tile to the sky for caves to exist
			// fill in water blocks
			float temperate = m_temperature[offset];
			while (terrainHeight <= (SIZE_Z >> 1))
			{
				if (GetBlock(x, y, terrainHeight) == AIR)
				{
					if (terrainHeight > Interpolate((SIZE_Z >> 1) - 3, (SIZE_Z >> 1) + 5, temperate))
					{
						SetBlock(x, y, terrainHeight, ICE);
					}
					else
					{
						SetBlock(x, y, terrainHeight, WATER);
					}
				}
				terrainHeight++;
			}
		}
	}

	// create trees
	CreateTrees(baseX, baseY);
	CreateVillage(m_chunkCoords.x, m_chunkCoords.y);

	m_status = ChunkState::CHUNK_COMPLETE;
	return true;
}

//--------------------------------------------------------------------------------
void Chunk::CopyTreeTemplateToWorld(BlockTemplate const* tree, int terrainHeight, int dx, int dy)
{
	for (int index = 0; index < (int)tree->m_blocks.size(); index++)
	{
		terrainHeight; // move up to base position
		IntVec3 position = tree->m_blocks[index].m_offset;
		position.x += dx;
		position.y += dy;
		position.z += terrainHeight;
		if (position.x < 0 || position.x > MASK_X)
		{
			continue;
		}
		if (position.y < 0 || position.y > MASK_Y)
		{
			continue;
		}
		if (position.z < 0 || position.z > MASK_Z)
		{
			continue;
		}

		SetBlock(position, tree->m_blocks[index].m_block);
	}
}

//--------------------------------------------------------------------------------
void Chunk::CreateTrees(int baseX, int baseY)
{
	for (int y = (TREE_DIAMETER >> 1); y < NOISE_DIM - (TREE_DIAMETER >> 1); y++) // 2..21 with 2 block buffer around square
	{
		for (int x = (TREE_DIAMETER >> 1); x < NOISE_DIM - (TREE_DIAMETER >> 1); x++)
		{
			if (TestTreeForBlock(x, y) == true)
			{
				int dx = (baseX + x - TREE_DIAMETER);
				int dy = (baseY + y - TREE_DIAMETER);
				int offset = y * NOISE_DIM + x;
				int terrainHeight;
				uint8_t block = DetermineSurfaceTerrain(offset, (float)dx, (float)dy, &terrainHeight);
				if (terrainHeight <= (SIZE_Z >> 1))
				{
					continue; // no trees underwater
				}
				dx -= baseX;
				dy -= baseY;

				if (block == SAND && m_humidity[offset] < HUMIDITY_LINE) // avoid trees on beaches unless desert
				{
					BlockTemplate const* tree;
					tree = BlockTemplate::GetByName("Cactus");
					if (tree)
					{
						CopyTreeTemplateToWorld(tree, terrainHeight, dx, dy);
					}
					else // do something to show the error
					{
						if (dx < 0 || dx > MASK_X)
							continue;
						if (dy < 0 || dy > MASK_Y)
							continue;
						SetBlock(dx, dy, ++terrainHeight, CACTUS);
						SetBlock(dx, dy, ++terrainHeight, CACTUS);
						SetBlock(dx, dy, ++terrainHeight, CACTUS);
						SetBlock(dx, dy, ++terrainHeight, CACTUS);
					}
				}
				if (block == GRASS)
				{
					if (m_humidity[offset] > 0.75)
					{
						BuildingTemplate* treeTemplate = BuildingTemplate::GetByName("swamptree");
						treeTemplate->CopyBuildingToChunk(this,  terrainHeight, dx, dy, 0);
						return;
					}
					BlockTemplate const* tree;
					if (m_temperature[offset] < 0.4f)
					{
						tree = BlockTemplate::GetByName("SpruceTree");
					}
					else
					{
						tree = BlockTemplate::GetByName("OakTree");
					}

					if (tree)
					{
						CopyTreeTemplateToWorld(tree, terrainHeight, dx, dy);					
					}
					else // do something to mark failure
					{
						if (dx < 0 || dx > MASK_X)
							continue;
						if (dy < 0 || dy > MASK_Y)
							continue;
						SetBlock(dx, dy, ++terrainHeight, GLOWSTONE);
						SetBlock(dx, dy, ++terrainHeight, GLOWSTONE);
						SetBlock(dx, dy, ++terrainHeight, GLOWSTONE);
						SetBlock(dx, dy, ++terrainHeight, GLOWSTONE);
					}
				}
			}
		}
	}
}

//--------------------------------------------------------------------------------
void Chunk::CreateVillage(int chunkX, int chunkY)
{
	for (int y = (VILLAGE_RANGE >> 1); y < (VILLAGE_RANGE >> 1) + VILLAGE_DIAMETER + 1; y++) // test VILLAGE_DIAMETER around chunk
	{
		for (int x = (VILLAGE_RANGE >> 1); x < (VILLAGE_RANGE >> 1) + VILLAGE_DIAMETER + 1; x++)
		{
			if (TestVillageForChunk(x, y) == true) // tests area around chunk to see if this chunk is a town center
			{
				// get world coordinates for chunk (0,0) block
				IntVec2 townChunk(chunkX + x - ((VILLAGE_RANGE + VILLAGE_DIAMETER) >> 1), chunkY + y - ((VILLAGE_RANGE + VILLAGE_DIAMETER) >> 1));
				int worldx = chunkX << BITS_X;
				int worldy = chunkY << BITS_Y;
				char msg[80];
				sprintf_s(msg, "Found town: %4i %4i", worldx + 7, worldy + 7);
				if (g_theGame->m_isDebug && chunkX == townChunk.x && chunkY == townChunk.y)
					DebugAddMessage(msg, 60.0f, Rgba8::RED, Rgba8::PURPLE);

				m_townType = 0; // Get2dNoiseUint(chunkX, chunkY, m_worldSeed + 8);
				// find average humidity and temperature in town area
				float humidity = 0.0f;
				float temperature = 0.0f;
				float tx = float(townChunk.x << BITS_X) + 7.0f;
				float ty = float(townChunk.y << BITS_Y) + 7.0f;
				for (int wx = -2; wx < 3; wx++)
				{
					for (int wy = -2; wy < 3; wy++)
					{
						humidity += 0.5f + 0.5f * Compute2dPerlinNoise(tx + 16.0f * wx, ty + 16.0f * wy, HUMIDITY_SCALE, 6, 0.5f, 2.0f, true, m_worldSeed + 1);
						temperature += 0.5f + 0.5f * Compute2dPerlinNoise(tx + 16.0f * wx, ty + 16.0f * wy, TEMP_SCALE, 8, 0.6f, 2.0f, true, m_worldSeed + 2);
					}
				}

				humidity *= 0.04f;
				temperature *= 0.04f;

				if (humidity < HUMIDITY_LINE)
				{
					m_townType = 1;
				}
				else if (temperature < 0.4f)
				{
					m_townType = 2;
				}

				// determine building type for chunk
// 				float buildingType = 0.5f + 0.5f * Compute2dPerlinNoise(float(chunkX), float(chunkY), 1.0f, 3, 0.5f, 2.0f, true, m_worldSeed + 10);
				int rotation = Get2dNoiseUint(chunkX, chunkY, m_worldSeed + 9) & 3; // mask off to get 4 cardinal choices * 90 degrees
				UNUSED(rotation);
				int buildingType = Get2dNoiseUint(chunkX, chunkY, m_worldSeed + 10) % 20; // mask off to 32 choices
				std::string name;
				switch (buildingType)
				{
				case 0:
				case 1:
					name = "hut";
					break;
				case 2:
				case 3:
				case 4:
					name = "house";
					break;
				case 5:
					name = "well";
					break;
				case 6:
				case 7:
					name = "shop";
					break;
				case 8:
				case 9:
					name = "forge";
					break;
				case 10:
					name = "church";
					break;
				case 11:
				case 12:
					name = "tower";
					break;
				default:
					return; // no building in this chunk after all
				}
				BuildingTemplate* buildingCandidate = BuildingTemplate::GetByName(name.c_str());

				// get footprint size and clear building pad
				IntVec2 footprint(Clamp(2 * buildingCandidate->m_xoffset + 1, 0, 15), Clamp(2 * buildingCandidate->m_yoffset + 1, 0, 15));

				// determine elevation of chunk for building (if elevation is too steep, can ignore the building spot!!!!!!!!!!!!!!!!!!!!!
				int terrainHeight;
				int offset;
				offset = (7 + TREE_DIAMETER) * NOISE_DIM + 7 + TREE_DIAMETER;
				DetermineSurfaceTerrain(offset, (float)(worldx + 7), (float)(worldy + 7), &terrainHeight);
				if (terrainHeight < SIZE_Z >> 1)
					return; // water in center of chunk stops building
				int sum = terrainHeight;
				int highest = terrainHeight;
				int lowest = terrainHeight;

				int dx = (16 - footprint.x) >> 1;
				int dy = (16 - footprint.y) >> 1;
				offset = (dy  + TREE_DIAMETER) * NOISE_DIM + dx  + TREE_DIAMETER;
				DetermineSurfaceTerrain(offset, (float)(worldx + dx), (float)(worldy + dy), &terrainHeight);
				sum += terrainHeight;
				if (terrainHeight > highest)
				{
					highest = terrainHeight;
				}
				if (terrainHeight < lowest)
				{
					lowest = terrainHeight;
				}

				dx = ((16 - footprint.x) >> 1) + footprint.x - 1;
				offset = (dy + TREE_DIAMETER) * NOISE_DIM + dx + TREE_DIAMETER;
				DetermineSurfaceTerrain(offset, (float)(worldx + dx), (float)(worldy + dy), &terrainHeight);
				sum += terrainHeight;
				if (terrainHeight > highest)
				{
					highest = terrainHeight;
				}
				if (terrainHeight < lowest)
				{
					lowest = terrainHeight;
				}

				dy = ((16 - footprint.y) >> 1) + footprint.y - 1;
				offset = (dy + TREE_DIAMETER) * NOISE_DIM + dx + TREE_DIAMETER;
				DetermineSurfaceTerrain(offset, (float)(worldx + dx), (float)(worldy + dy), &terrainHeight);
				sum += terrainHeight;
				if (terrainHeight > highest)
				{
					highest = terrainHeight;
				}
				if (terrainHeight < lowest)
				{
					lowest = terrainHeight;
				}

				dx = (16 - footprint.x) >> 1;
				offset = (dy + TREE_DIAMETER) * NOISE_DIM + dx + TREE_DIAMETER;
				DetermineSurfaceTerrain(offset, (float)(worldx + dx), (float)(worldy + dy), &terrainHeight);
				sum += terrainHeight;
				if (terrainHeight > highest)
				{
					highest = terrainHeight;
				}
				if (terrainHeight < lowest)
				{
					lowest = terrainHeight;
				}

				if ((highest - lowest > footprint.x) || (highest - lowest > footprint.y))
				{
					return; // no buildings on steep slopes
				}
				float averageHeight = float(sum) / 5.0f; // average of four corners and middle
				if (averageHeight < float(SIZE_Z >> 1))
				{
					return; // no buildings below sea level
				}
				if (averageHeight > 110.0f)
				{
					return; // no buildings that might hit build limit
				}

				// flatten out the footprint for the building
				int nudgex = Get2dNoiseUint(chunkX, chunkY, m_worldSeed + 11);
				nudgex %= ((16 - footprint.x) >> 1) + 1;
				nudgex *= Get2dNoiseUint(chunkX + 1, chunkY, m_worldSeed + 11) % 2 ? -1 : 1;
				int nudgey = Get2dNoiseUint(chunkX, chunkY, m_worldSeed + 12);
				nudgey %= ((16 - footprint.y) >> 1) + 1;
				nudgey *= Get2dNoiseUint(chunkX, chunkY + 1, m_worldSeed + 12) % 2 ? -1 : 1;

				for (int i = ((16 - footprint.x) >> 1) + nudgex; i < ((16 - footprint.x) >> 1) + footprint.x + nudgex; i++)
				{
					for (int j = ((16 - footprint.y) >> 1) + nudgey; j < ((16 - footprint.y) >> 1) + footprint.y + nudgey; j++)
					{
						int elevation = MASK_Z;
						while( elevation > int(averageHeight))
						{
							SetBlock(i, j, elevation, AIR);
							elevation--;
						}
						offset = (j + TREE_DIAMETER) * NOISE_DIM + i + TREE_DIAMETER;
						uint8_t block = DetermineSurfaceTerrain(offset, (float)(worldx + i), (float)(worldy + j), &terrainHeight);
						SetBlock(i, j, elevation, block);
						elevation--;
						if (block == GRASS)
						{
							block = DIRT;
						}
						while (elevation >= terrainHeight)
						{
							SetBlock(i, j, elevation, block);
							elevation--;
						}
					}
				}

				// copy building template with selected orientation and offset at the build height
				buildingCandidate->CopyBuildingToChunk(this,  int(averageHeight), 7 + nudgex, 7 + nudgey, rotation);
			}
		}
	}
}

//--------------------------------------------------------------------------------
// offset is into noise array and dx/dy are world coordinates for column
uint8_t Chunk::DetermineSurfaceTerrain(int offset, float dx, float dy, int*  terrainHeight)
{
	// calculate base terrain noise level
	float baseNoise = Compute2dPerlinNoise(dx, dy, NOISE_SCALE, NOISE_OCTAVES, 0.5f, 2.0f, true, m_worldSeed);

	// add hilliness factor
	float hilly = SmoothStep3(m_hilliness[offset]);
	baseNoise *= hilly;

	// convert to rivers
	float baseHeight = RangeMap(Hesitate3(fabsf(baseNoise)), 0.0f, 1.0f, RIVER_FLOOR, 126.0f);
// 	float baseHeight = RangeMap(SmoothStop3(fabsf(baseNoise)), 0.0f, 1.0f, RIVER_FLOOR, 126.0f);
// 	float baseHeight = RangeMap((fabsf(baseNoise)), 0.0f, 1.0f, 60.0f, 126.0f);

	*terrainHeight = int(baseHeight);

	// create oceans
	float oceaniness = m_ocean[offset];
	if (oceaniness > OCEAN_FLOOR)
	{
		*terrainHeight = MAX_OCEAN_DEPTH;
	}
	else if (oceaniness > OCEAN_LERPED)
	{
		*terrainHeight = Interpolate(*terrainHeight, MAX_OCEAN_DEPTH, oceaniness * OCEAN_RANGE);
	}
	// else leave terrain unmodified

	// determine terrain type based on humidity and temperature
	float humid = m_humidity[offset];

	if (humid < HUMIDITY_LINE || (*terrainHeight == SIZE_Z >> 1 && humid < BEACH_LINE))
	{
		return SAND;
	}
	else
	{
		return GRASS;
	}
}

bool Chunk::TestTreeForBlock(int x, int y)
{
	int index = y * NOISE_DIM + x;
	for (int ty = 0; ty < TREE_DIAMETER + 1; ty++)
	{
		for (int tx = 0; tx < TREE_DIAMETER + 1; tx++)
		{
			int offset = (y - (TREE_DIAMETER >> 1) + ty) * NOISE_DIM + (x - (TREE_DIAMETER >> 1) + tx);
			if (m_forest[offset] > m_forest[index])
				return false;
		}
	}
	return true;
}

bool Chunk::TestVillageForChunk(int x, int y)
{
	constexpr int SEARCH_RANGE = VILLAGE_RANGE >> 1;
	// x and y are offsets to village area around chunk
	int offset;
	int index = y * NOISE_DIM + x;
	for (int ty = 0; ty < SEARCH_RANGE + 1; ty++)
	{
		for (int tx = 0; tx < SEARCH_RANGE + 1; tx++)
		{
//			offset = (y - (VILLAGE_RANGE >> 1) + ty) * NOISE_DIM + (x - (VILLAGE_RANGE >> 1) + tx);
			offset = (y - (SEARCH_RANGE >> 1) + ty) * NOISE_DIM + (x - (SEARCH_RANGE >> 1) + tx);
			if (m_town[offset] > m_town[index])
				return false;
		}
	}
	return true;
}

uint8_t Chunk::GetBlock(int x, int y, int z)
{
	return GetBlock(x + y * SIZE_X + z * BLOCKSPERLAYER);
}

uint8_t Chunk::GetBlock(int index)
{
	return m_block[index].GetBlockDefinition();
}

Block* Chunk::GetBlockPtr(int index)
{
	return &m_block[index];
}

void Chunk::SetBlock(IntVec3 position, uint8_t value)
{
	SetBlock(position.x, position.y, position.z, value);
}

void Chunk::SetBlock(int x, int y, int z, uint8_t value)
{
	SetBlock(x + y * SIZE_X + z * BLOCKSPERLAYER, value);
}

void Chunk::SetBlock(int index, uint8_t value)
{
	if (index >= 0 && index < BLOCKSPERCHUNK)
	{
		m_block[index].SetBlockDefinition(value);
		m_block[index].InitializeFlags();
	}
}

void Chunk::CreateBuffers()
{
	if (!indexedDraw || m_indexCount == 0) // don't create an index buffer if there are no vertices to draw
	{
		return;
	}

	if (m_immediateVBO_PCU)
	{
		delete m_immediateVBO_PCU;
	}

	if (m_indexBuffer)
	{
		delete m_indexBuffer;
	}

	m_indexBuffer = g_theRenderer->CreateIndexBuffer(m_indexes.data(), m_indexCount * sizeof(unsigned int)); // multiply by  to get total count?
	m_immediateVBO_PCU = g_theRenderer->CreateVertexBuffer(sizeof(Vertex_PCU), sizeof(Vertex_PCU));

	g_theRenderer->CopyCPUToGPU(m_vertexes.data(), static_cast<size_t>(m_vertexes.size()) * m_immediateVBO_PCU->GetStride(), m_immediateVBO_PCU);
	g_theRenderer->CopyCPUToGPU(m_indexes.data(), static_cast<size_t>(m_indexes.size()) * m_indexBuffer->GetStride(), m_indexBuffer);
}

void Chunk::CreateGeometry()
{
	Rgba8 zColor = Rgba8::WHITE;
	Rgba8 yColor = Rgba8(205, 205, 205);
	Rgba8 xColor = Rgba8(230, 230, 230);
	m_indexes.clear();
	m_vertexes.clear();

	if (indexedDraw)
	{
		for (int index = 0; index < BLOCKSPERCHUNK; index++)
		{
			AABB3 bounds = GetBlockBounds(index);
			Vec3 c000(bounds.m_mins.x, bounds.m_mins.y, bounds.m_mins.z);
			Vec3 c001(bounds.m_mins.x, bounds.m_mins.y, bounds.m_maxs.z);
			Vec3 c010(bounds.m_mins.x, bounds.m_maxs.y, bounds.m_mins.z);
			Vec3 c011(bounds.m_mins.x, bounds.m_maxs.y, bounds.m_maxs.z);
			Vec3 c100(bounds.m_maxs.x, bounds.m_mins.y, bounds.m_mins.z);
			Vec3 c101(bounds.m_maxs.x, bounds.m_mins.y, bounds.m_maxs.z);
			Vec3 c110(bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_mins.z);
			Vec3 c111(bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_maxs.z);

			BlockIterator block(this, index);
			if (BlockDefinition::s_definitions[GetBlock(index)].m_visible)
			{
				AABB2 UVs;
				if (IsVisible(index, 0)) // top
				{
					UVs = BlockDefinition::s_definitions[GetBlock(index)].m_top_uvs;
					zColor = BlockFaceLight(block, 0);
					AddVertsForQuad3D(m_indexes, m_vertexes, c011, c001, c101, c111, zColor, UVs);
				}

				UVs = BlockDefinition::s_definitions[GetBlock(index)].m_side_uvs;
				if (IsVisible(index, 1)) // south
				{
					xColor = BlockFaceLight(block, 1);
					AddVertsForQuad3D(m_indexes, m_vertexes, c001, c000, c100, c101, xColor, UVs);
				}
				if (IsVisible(index, 2)) // east
				{
					yColor = BlockFaceLight(block, 2);
					AddVertsForQuad3D(m_indexes, m_vertexes, c101, c100, c110, c111, yColor, UVs);
				}
				if (IsVisible(index, 3)) // north
				{
					xColor = BlockFaceLight(block, 3);
					AddVertsForQuad3D(m_indexes, m_vertexes, c111, c110, c010, c011, xColor, UVs);
				}
				if (IsVisible(index, 4)) // west
				{
					yColor = BlockFaceLight(block, 4);
					AddVertsForQuad3D(m_indexes, m_vertexes, c011, c010, c000, c001, yColor, UVs);
				}

				if (IsVisible(index, 5)) // bottom
				{
					UVs = BlockDefinition::s_definitions[GetBlock(index)].m_bottom_uvs;
					zColor = BlockFaceLight(block, 5);
					AddVertsForQuad3D(m_indexes, m_vertexes, c000, c010, c110, c100, zColor, UVs);
				}
			}
		}

		m_indexCount = static_cast<unsigned int>(m_indexes.size()); // could eliminate this
		return;
	}

	// non-indexed buffer draw for testing
	for (int index = 0; index < BLOCKSPERCHUNK; index++)
	{
		AABB3 bounds = GetBlockBounds(index);
// 		AddVertsForAABB3D(m_vertexes, bounds, color);
// 		continue;

		Vec3 c000(bounds.m_mins.x, bounds.m_mins.y, bounds.m_mins.z);
		Vec3 c001(bounds.m_mins.x, bounds.m_mins.y, bounds.m_maxs.z);
		Vec3 c010(bounds.m_mins.x, bounds.m_maxs.y, bounds.m_mins.z);
		Vec3 c011(bounds.m_mins.x, bounds.m_maxs.y, bounds.m_maxs.z);
		Vec3 c100(bounds.m_maxs.x, bounds.m_mins.y, bounds.m_mins.z);
		Vec3 c101(bounds.m_maxs.x, bounds.m_mins.y, bounds.m_maxs.z);
		Vec3 c110(bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_mins.z);
		Vec3 c111(bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_maxs.z);

		if (BlockDefinition::s_definitions[GetBlock(index)].m_visible)
		{
			AABB2 UVs = BlockDefinition::s_definitions[GetBlock(index)].m_top_uvs;
			AddVertsForQuad3D(m_vertexes, c011, c111, c101, c001, zColor, UVs);

			UVs = BlockDefinition::s_definitions[GetBlock(index)].m_side_uvs;
			AddVertsForQuad3D(m_vertexes, c001, c000, c100, c101, xColor, UVs);
			AddVertsForQuad3D(m_vertexes, c101, c100, c110, c111, yColor, UVs);
			AddVertsForQuad3D(m_vertexes, c111, c110, c010, c011, xColor, UVs);
			AddVertsForQuad3D(m_vertexes, c011, c010, c000, c001, yColor, UVs);

			UVs = BlockDefinition::s_definitions[GetBlock(index)].m_bottom_uvs;
			AddVertsForQuad3D(m_vertexes, c000, c100, c110, c010, zColor, UVs);
		}
	}
}

Rgba8 Chunk::BlockFaceLight(BlockIterator block, int face)
{
	Rgba8 color = Rgba8(0, 0, 127);
	switch (face)
	{
	case 0:
		if (block.GetTopNeighbor().GetBlock())
		{
			color.r = (uint8_t)RangeMap(block.GetTopNeighbor().GetBlock()->GetOutdoorLight(), 0, MAX_LIGHT, 0, 255);
			color.g = (uint8_t)RangeMap(block.GetTopNeighbor().GetBlock()->GetIndoorLight(), 0, MAX_LIGHT, 0, 255);
		}
		else
		{
			return Rgba8::GREEN;
		}
		break;
	case 1:
		if (block.GetSouthNeighbor().GetBlock())
		{
			color.r = (uint8_t)RangeMap(block.GetSouthNeighbor().GetBlock()->GetOutdoorLight(), 0, MAX_LIGHT, 0, 255);
			color.g = (uint8_t)RangeMap(block.GetSouthNeighbor().GetBlock()->GetIndoorLight(), 0, MAX_LIGHT, 0, 255);
		}
		else
		{
			return Rgba8::GREEN;
		}
		break;
	case 2:
		if (block.GetEastNeighbor().GetBlock())
		{
			color.r = (uint8_t)RangeMap(block.GetEastNeighbor().GetBlock()->GetOutdoorLight(), 0, MAX_LIGHT, 0, 255);
			color.g = (uint8_t)RangeMap(block.GetEastNeighbor().GetBlock()->GetIndoorLight(), 0, MAX_LIGHT, 0, 255);
		}
		else
		{
			return Rgba8::GREEN;
		}
		break;
	case 3:
		if (block.GetNorthNeighbor().GetBlock())
		{
			color.r = (uint8_t)RangeMap(block.GetNorthNeighbor().GetBlock()->GetOutdoorLight(), 0, MAX_LIGHT, 0, 255);
			color.g = (uint8_t)RangeMap(block.GetNorthNeighbor().GetBlock()->GetIndoorLight(), 0, MAX_LIGHT, 0, 255);
		}
		else
		{
			return Rgba8::GREEN;
		}
		break;
	case 4:
		if (block.GetWestNeighbor().GetBlock())
		{
			color.r = (uint8_t)RangeMap(block.GetWestNeighbor().GetBlock()->GetOutdoorLight(), 0, MAX_LIGHT, 0, 255);
			color.g = (uint8_t)RangeMap(block.GetWestNeighbor().GetBlock()->GetIndoorLight(), 0, MAX_LIGHT, 0, 255);
		}
		else
		{
			return Rgba8::GREEN;
		}
		break;
	case 5:
		if (block.GetBottomNeighbor().GetBlock())
		{
			color.r = (uint8_t)RangeMap(block.GetBottomNeighbor().GetBlock()->GetOutdoorLight(), 0, MAX_LIGHT, 0, 255);
			color.g = (uint8_t)RangeMap(block.GetBottomNeighbor().GetBlock()->GetIndoorLight(), 0, MAX_LIGHT, 0, 255);
		}
		else
		{
			return Rgba8::GREEN;
		}
		break;
	}
	return color; // to make compiler happy
}

bool Chunk::IsVisible(int index, int face)
{
	bool visible = false;
	int x = index & MASK_X;
	int y = (index >> BITS_X) & MASK_Y;
	int z = (index >> (BITS_X + BITS_Y)) & MASK_Z;

	switch (face)
	{
	case 0:  // top
		if (z == MASK_Z)
		{
			visible = true;
		}
		else
		{
			visible = !BlockDefinition::s_definitions[GetBlock(x, y, z + 1)].m_visible;
		}
		break;
	case 1:  // south
		if (y == 0)
		{
			visible = m_neighbors[SOUTH] && !BlockDefinition::s_definitions[m_neighbors[SOUTH]->GetBlock(x, MASK_Y, z)].m_visible;
		}
		else
		{
			visible = !BlockDefinition::s_definitions[GetBlock(x, y - 1, z)].m_visible;
		}
		break;
	case 2:  // east
		if (x == MASK_X)
		{
			visible = m_neighbors[EAST] && !BlockDefinition::s_definitions[m_neighbors[EAST]->GetBlock(0, y, z)].m_visible;
		}
		else
		{
			visible = !BlockDefinition::s_definitions[GetBlock(x + 1, y, z)].m_visible;
		}		
		break;
	case 3:  // north
		if (y == MASK_Y)
		{
			visible = m_neighbors[NORTH] && !BlockDefinition::s_definitions[m_neighbors[NORTH]->GetBlock(x, 0, z)].m_visible;
		}
		else
		{
			visible = !BlockDefinition::s_definitions[GetBlock(x, y + 1, z)].m_visible;
		}		
		break;
	case 4:  // west
		if (x == 0)
		{
			visible = m_neighbors[WEST] && !BlockDefinition::s_definitions[m_neighbors[WEST]->GetBlock(MASK_X, y, z)].m_visible;
		}
		else
		{
			visible = !BlockDefinition::s_definitions[GetBlock(x - 1, y, z)].m_visible;
		}
		break;
	case 5:  // bottom
		if (z == 0)
		{
			visible = true;
		}
		else
		{
			visible = !BlockDefinition::s_definitions[GetBlock(x, y, z - 1)].m_visible;
		}
		break;
	}
	return visible;
}

AABB3 Chunk::GetBlockBounds(int index)
{
	int x = index & MASK_X;
	int y = (index >> BITS_X) & MASK_Y;
	int z = (index >> (BITS_X + BITS_Y)) & MASK_Z;
	return GetBlockBounds(x, y, z);
}

AABB3 Chunk::GetBlockBounds(int x, int y, int z)
{
	AABB3 bounds;
	bounds.m_mins.x = m_worldBounds.m_mins.x + (float)x;
	bounds.m_mins.y = m_worldBounds.m_mins.y + (float)y;
	bounds.m_mins.z = m_worldBounds.m_mins.z + (float)z;
	bounds.m_maxs.x = m_worldBounds.m_mins.x + (float)x + 1.0f;
	bounds.m_maxs.y = m_worldBounds.m_mins.y + (float)y + 1.0f;
	bounds.m_maxs.z = m_worldBounds.m_mins.z + (float)z + 1.0f;
	return bounds;
}

void Chunk::Update(float deltaSeconds)
{
	UNUSED(deltaSeconds);
}

void Chunk::Render()
{
	for (int index = 0; index < 4; index++)
	{
		if (m_neighbors[index] == nullptr)
		{
			return; // do not render this chunk unless all four neighbors are present
		}
	}

	Texture* terrainTexture = g_theRenderer->CreateOrGetTextureFromFile((char const*)"Data/Images/BasicSprites_64x64.png");
	g_theRenderer->BindTexture(terrainTexture);
	g_theRenderer->SetModelMatrix(Mat44());
	g_theRenderer->BindShaderByName("Data/Shaders/World");

	if (indexedDraw)
	{
		if (m_indexBuffer) // only draw if there is something to draw (an empty chunk has no vertices to draw)
		{
			g_theRenderer->DrawIndexedVertexBuffer(m_indexBuffer, m_immediateVBO_PCU, m_indexCount);
		}
	}
	else
	{
		g_theRenderer->DrawVertexArray(int(m_vertexes.size()), &m_vertexes[0]);
	}
	g_theRenderer->BindShader(nullptr);
	g_theRenderer->BindTexture(nullptr);
}

void Chunk::WriteChunkToDisc()
{
	std::vector<uint8_t> outBuffer = {};
	outBuffer.push_back('G');
	outBuffer.push_back('C');
	outBuffer.push_back('H');
	outBuffer.push_back('K');
	outBuffer.push_back(1);
	outBuffer.push_back(4);
	outBuffer.push_back(4);
	outBuffer.push_back(7);

	// Concatenate RLE runs
	uint8_t blocktype = m_block[0].GetBlockDefinition();
	uint8_t count = 1;
	for (int index = 1; index < BLOCKSPERCHUNK; index++)
	{
		if (m_block[index].GetBlockDefinition() == blocktype)
		{
			count++;
			if (count == 255)
			{
				outBuffer.push_back(blocktype);
				outBuffer.push_back(count);
				count = 0;
			}
		}
		else
		{
			if (count > 0)
			{
				outBuffer.push_back(blocktype);
				outBuffer.push_back(count);
			}
			blocktype = m_block[index].GetBlockDefinition();
			count = 1;
		}
	}
	// write out last set, if any
	if (count > 0)
	{
		outBuffer.push_back(blocktype);
		outBuffer.push_back(count);
	}

	char filename[80];
	sprintf_s(filename, "%s/Chunk(%i,%i).chunk", g_theGame->m_world->m_path.c_str(), m_chunkCoords.x, m_chunkCoords.y);
	if (FileWriteBinaryBuffer(outBuffer, filename))
	{
		DebuggerPrintf("Error writing chunk [%f, %f]\n", m_chunkCoords.x, m_chunkCoords.y);
	}
}

void Chunk::ReadChunkFromDisc(const char* filename)
{
	static uint8_t header[] = { 'G', 'C', 'H', 'K', 1, 4, 4, 7 };
	std::vector<uint8_t> outBuffer = {};
// 	char filename[80];
// 	sprintf_s(filename, "Saves/Chunk(%i,%i).chunk", m_chunkCoords.x, m_chunkCoords.y);
	if (FileReadToBuffer(outBuffer, filename))
	{
		DebuggerPrintf("Error reading chunk [%f, %f]\n", m_chunkCoords.x, m_chunkCoords.y);
	}

	// check for correct header
	for (int index = 0; index < 8; index++)
	{
		if (outBuffer[index] != header[index])
		{
			DebuggerPrintf("Error in chunk header [%f, %f]\n", m_chunkCoords.x, m_chunkCoords.y);
			ERROR_AND_DIE("Bad header on chunk");
		}
	}

	int index = 0;
	uint8_t blockType = AIR;
	uint8_t count = 0;
	for (int offset = 8; offset < (int)outBuffer.size(); offset += 2)
	{
		blockType = outBuffer[offset];
		count = outBuffer[offset + 1];
		for (int i = 0; i < count; i++)
		{
			m_block[index].SetBlockDefinition(blockType);
			m_block[index].InitializeFlags();
			index++;
		}
	}
	if (index != BLOCKSPERCHUNK)
	{
	DebuggerPrintf("Error decoding chunk data [%f, %f] index = %i\n", m_chunkCoords.x, m_chunkCoords.y, index);
	ERROR_AND_DIE("Bad data for chunk");
	}
}

void Chunk::LinkNeighbors(World const& world)
{
	Chunk* neighbor = nullptr;
	neighbor = world.GetMappedValue(m_chunkCoords.x, m_chunkCoords.y + 1);
	if (neighbor)
	{
		neighbor->m_neighbors[SOUTH] = this;
		m_neighbors[NORTH] = neighbor;
		neighbor->m_needsMesh = true;
		m_needsMesh = true;
	}
	else
	{
		m_neighbors[NORTH] = nullptr;
		m_needsMesh = true;
	}

	neighbor = world.GetMappedValue(m_chunkCoords.x + 1, m_chunkCoords.y);
	if (neighbor)
	{
		neighbor->m_neighbors[WEST] = this;
		m_neighbors[EAST] = neighbor;
		neighbor->m_needsMesh = true;
		m_needsMesh = true;
	}
	else
	{
		m_neighbors[EAST] = nullptr;
		m_needsMesh = true;
	}

	neighbor = world.GetMappedValue(m_chunkCoords.x, m_chunkCoords.y - 1);
	if (neighbor)
	{
		neighbor->m_neighbors[NORTH] = this;
		m_neighbors[SOUTH] = neighbor;
		neighbor->m_needsMesh = true;
		m_needsMesh = true;
	}
	else
	{
		m_neighbors[SOUTH] = nullptr;
		m_needsMesh = true;
	}

	neighbor = world.GetMappedValue(m_chunkCoords.x - 1, m_chunkCoords.y);
	if (neighbor)
	{
		neighbor->m_neighbors[EAST] = this;
		m_neighbors[WEST] = neighbor;
		neighbor->m_needsMesh = true;
		m_needsMesh = true;
	}
	else
	{
		m_neighbors[WEST] = nullptr;
		m_needsMesh = true;
	}
}

void Chunk::Initialize(IntVec2 worldChunkCoords)
{
	m_chunkCoords = worldChunkCoords;
	m_worldBounds.m_mins.x = (float)(worldChunkCoords.x << BITS_X);
	m_worldBounds.m_mins.y = (float)(worldChunkCoords.y << BITS_Y);
	m_worldBounds.m_mins.z = 0.0f;
	m_worldBounds.m_maxs.x = m_worldBounds.m_mins.x + (float)MASK_X;
	m_worldBounds.m_maxs.y = m_worldBounds.m_mins.y + (float)MASK_Y;
	m_worldBounds.m_maxs.z = (float)MASK_Z;
}

void Chunk::Activate(World & world)
	{
	// create basic rendering stuff for now
	// m_dirty = false;
	m_needsMesh = true; // starts dirty to force mesh generation
	LinkNeighbors(world);

	// set block lighting
	int index = 0;
	for (int y = 0; y < SIZE_Y; y++)
	{
		for (int x = 0; x < SIZE_X; x++)
		{
			int z = MASK_Z;
			while (z >= 0 && GetBlock(x, y, z) == AIR)
			{
				index = z << (BITS_X + BITS_Y) | y << BITS_X | x;
				m_block[index].SetSky(true);
				z--;
			}
			// mark all non-opaque edge blocks as light dirty (including sky blocks)
			if ((x == 0 && m_neighbors[WEST]) || (x == MASK_X && m_neighbors[EAST]) || (y == 0 && m_neighbors[SOUTH]) || (y == MASK_Y && m_neighbors[NORTH]))
			{
				for (z = MASK_Z; z >= 0; z--)
				{
					index = z << (BITS_X + BITS_Y) | y << BITS_X | x;
					if (m_block[index].IsOpaque() == false)
					{
						world.MarkLightingDirty(this, index);
					}
				}
			}
		}
	}

	for (int y = 0; y < SIZE_Y; y++)
	{
		for (int x = 0; x < SIZE_X; x++)
		{
			int z = MASK_Z;
			// mark sky blocks as maximum light intensity
			while (z >= 0 && GetBlock(x, y, z) == AIR)
			{
				index = z << (BITS_X + BITS_Y) | y << BITS_X | x;
				m_block[index].SetOutdoorLight(MAX_LIGHT);
				// mark neighbors dirty
				if (x == 0)
				{
					if (m_neighbors[WEST])
					{
						index = z << (BITS_X + BITS_Y) | y << BITS_X | MASK_X;
						world.MarkLightingDirty(m_neighbors[WEST], index);
					}
				}
				else
				{
					index = z << (BITS_X + BITS_Y) | y << BITS_X | (x - 1);
					world.MarkLightingDirty(this, index);
				}

				if (x == MASK_X)
				{
					if (m_neighbors[EAST])
					{
						index = z << (BITS_X + BITS_Y) | y << BITS_X | 0;
						world.MarkLightingDirty(m_neighbors[EAST], index);
					}
				}
				else
				{
					index = z << (BITS_X + BITS_Y) | y << BITS_X | (x + 1);
					world.MarkLightingDirty(this, index);
				}

				if (y == 0)
				{
					if (m_neighbors[SOUTH])
					{
						index = z << (BITS_X + BITS_Y) | MASK_X << BITS_X | x;
						world.MarkLightingDirty(m_neighbors[SOUTH], index);
					}
				}
				else
				{
					index = z << (BITS_X + BITS_Y) | (y - 1) << BITS_Y | x;
					world.MarkLightingDirty(this, index);
				}

				if (y == MASK_Y)
				{
					if (m_neighbors[NORTH])
					{
						index = z << (BITS_X + BITS_Y) | 0 << BITS_Y | x;
						world.MarkLightingDirty(m_neighbors[NORTH], index);
					}
				}
				else
				{
					index = z << (BITS_X + BITS_Y) | (y + 1) << BITS_Y | x;
					world.MarkLightingDirty(this, index);
				}

				z--;
			}

			// mark light-emitting blocks dirty
			for (; z >= 0; z--)
			{
				index = z << (BITS_X + BITS_Y) | y << BITS_X | x;
				if (m_block[index].GetLightEmitted() > 0)
				{
					world.MarkLightingDirty(this, index);
				}
 			}
		}
	}

}

void Chunk::Deactivate()
{
	if (m_dirty)
	{
		WriteChunkToDisc();
	}
}

// static functions
IntVec2 Chunk::GetChunkForWorldPosition(Vec3 position)
{
	int x = (int)(floor(position.x)) >> BITS_X;
	int y = (int)(floor(position.y)) >> BITS_Y;
	return IntVec2(x, y);
}

int Chunk::GetBlockForPosition(Vec3 position)
{
	int x = (int)(floor(position.x)) & MASK_X;
	int y = (int)(floor(position.y)) & MASK_Y;
	int z = (int)(floor(position.z));
	return z << (BITS_X + BITS_Y) | y << BITS_X | x;
}

AABB2 Chunk::GetChunkWorldBounds(int x, int y)
{
	AABB2 bounds;
	bounds.m_mins.x = (float)(x << BITS_X);
	bounds.m_mins.y = (float)(y << BITS_Y);
	bounds.m_maxs.x = bounds.m_mins.x + (float)MASK_X;
	bounds.m_maxs.y = bounds.m_mins.y + (float)MASK_Y;
	return bounds;
}

void Chunk::TestNeighborNeedsMesh(int x, int y)
{
	if (x == 0 && m_neighbors[WEST])
	{
		m_neighbors[WEST]->m_needsMesh = true;
	}
	if (x == MASK_X && m_neighbors[EAST])
	{
		m_neighbors[EAST]->m_needsMesh = true;
	}
	if (y == 0 && m_neighbors[SOUTH])
	{
		m_neighbors[SOUTH]->m_needsMesh = true;
	}
	if (y == MASK_Y && m_neighbors[NORTH])
	{
		m_neighbors[NORTH]->m_needsMesh = true;
	}
}

uint8_t Chunk::ConvertToBlock(BuildingBlock variableBlock)
{
	// setting up arrays for larger number of choices would be good some day
	uint8_t actualBlock = AIR;
	switch (m_townType)
	{
	case 0: // ordinary town
		switch (variableBlock)
		{
		case BuildingBlock::EMPTY:
			actualBlock = AIR;
			break;
		case BuildingBlock::WALL:
			actualBlock = OAKPLANKS;
			break;
		case BuildingBlock::BEAM:
			actualBlock = OAKLOG;
			break;
		case BuildingBlock::FLOOR:
			actualBlock = IRREGULAR;
			break;
		case BuildingBlock::ROOF:
			actualBlock = ROOFPLANKS;
			break;
		case BuildingBlock::BRICK:
			actualBlock = REDBRICKS;
			break;
		case BuildingBlock::LIGHT:
			actualBlock = GLOWSTONE;
			break;
		case BuildingBlock::STONE:
			actualBlock = BRICKS;
			break;
		case BuildingBlock::WATER:
			actualBlock = WATER;
			break;
		case BuildingBlock::LOG:
			actualBlock = SWAMPLOG;
			break;
		case BuildingBlock::LEAVES:
			actualBlock = SWAMPLEAVES;
			break;
		}
		break;

	case 1: // desert town
		switch (variableBlock)
		{
		case BuildingBlock::EMPTY:
			actualBlock = AIR;
			break;
		case BuildingBlock::WALL:
			actualBlock = SANDSTONE;
			break;
		case BuildingBlock::BEAM:
			actualBlock = STONE;
			break;
		case BuildingBlock::FLOOR:
			actualBlock = GOLD;
			break;
		case BuildingBlock::ROOF:
			actualBlock = OAKPLANKS;
			break;
		case BuildingBlock::BRICK:
			actualBlock = IRREGULAR;
			break;
		case BuildingBlock::LIGHT:
			actualBlock = GLOWSTONE;
			break;
		case BuildingBlock::STONE:
			actualBlock = REDSANDSTONE;
			break;
		case BuildingBlock::WATER:
			actualBlock = WATER;
			break;
		}
		break;

	case 2: // arctic town
		switch (variableBlock)
		{
		case BuildingBlock::EMPTY:
			actualBlock = AIR;
			break;
		case BuildingBlock::WALL:
			actualBlock = SPRUCEPLANKS;
			break;
		case BuildingBlock::BEAM:
			actualBlock = SPRUCELOG;
			break;
		case BuildingBlock::FLOOR:
			actualBlock = DIAMOND;
			break;
		case BuildingBlock::ROOF:
			actualBlock = SPRUCELEAVES;
			break;
		case BuildingBlock::BRICK:
			actualBlock = WHITEBRICKS;
			break;
		case BuildingBlock::LIGHT:
			actualBlock = GLOWSTONE;
			break;
		case BuildingBlock::STONE:
			actualBlock = BLUESTONE;
			break;
		case BuildingBlock::WATER:
			actualBlock = WATER;
			break;
		}
		break;
	}


	return actualBlock;
}

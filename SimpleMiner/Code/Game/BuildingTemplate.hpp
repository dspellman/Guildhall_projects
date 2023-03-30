#pragma once
#include "Game/GameCommon.hpp"
#include <vector>

class Chunk;

enum class BuildingBlock
{
	EMPTY,
	WALL,
	BEAM,
	FLOOR,
	ROOF,
	BRICK,
	LIGHT,
	STONE,
	LOG,
	LEAVES,
	WATER,
};

struct BlockPosition
{
	IntVec3 position = IntVec3::ZERO;
	BuildingBlock type = BuildingBlock::EMPTY;
};

class BuildingTemplate
{
public:
	~BuildingTemplate();
	BuildingTemplate(const char* filename);
	void CopyBuildingToChunk(Chunk* chunk, int terrainHeight, int dx, int dy, int rotation);

	static void Initialize(const char* source);
	static void Destroy();
	static BuildingTemplate* GetByName(const std::string& name);
	static std::vector<BuildingTemplate*> s_buildingTemplates;

	int m_xoffset = 0;
	int m_yoffset = 0;

private:
	std::string m_name;
	std::vector<BlockPosition> m_blocks;
};
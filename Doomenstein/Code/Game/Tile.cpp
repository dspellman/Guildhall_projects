#include "Tile.hpp"
#include "Engine/Math/AABB3.hpp"
#include "TileDefinition.hpp"
#include "TileMaterialDefinition.hpp"

Tile::Tile(AABB3 bounds, const TileDefinition* definition /*= nullptr */)
	: m_bounds(bounds), m_definition(definition)
{
}

bool Tile::IsAir() const
{
	
	return m_definition->m_name.compare("air") == 0;
}

bool Tile::IsSolid() const
{
	return m_definition->m_isSolid;
}

bool Tile::HasFloor() const
{
	return m_definition->m_floorMaterialDefinition->m_name.compare("None");
}


#include "Engine/Core/TileHeatMap.hpp"

TileHeatMap::TileHeatMap(IntVec2 const& dimensions)
	: m_dimensions(dimensions)
{
	for (int index = 0; index < m_dimensions.x * m_dimensions.y; index++)
	{
		m_values.push_back(0.0f);
	}
}

void TileHeatMap::SetAllValues(float fillValue)
{
	for (int index = 0; index < m_dimensions.x * m_dimensions.y; index++)
	{
		m_values[index] = fillValue;
	}
}

void TileHeatMap::FloodFill(int x, int y, float value)
{
	Set(value, x, y);  // mark the current tile visited

	if (Get(x + 1, y) == 0.0f)
	{
		FloodFill(x + 1, y, value);
	}

	if (Get(x - 1, y) == 0.0f)
	{
		FloodFill(x - 1, y, value);
	}

	if (Get(x, y + 1) == 0.0f)
	{
		FloodFill(x, y + 1, value);
	}

	if (Get(x, y - 1) == 0.0f)
	{
		FloodFill(x, y - 1, value);
	}
}

float TileHeatMap::Get(IntVec2 const coords) const
{
	return m_values[coords.x + coords.y * m_dimensions.x];
}

float TileHeatMap::Get(int x, int y) const
{
	return m_values[x + y * m_dimensions.x];
}

void TileHeatMap::Set(float value, IntVec2 coords)
{
	m_values[coords.x + coords.y * m_dimensions.x] = value;
}

void TileHeatMap::Set(float value, int x, int y)
{
	m_values[x + y * m_dimensions.x] = value;
}

void TileHeatMap::Add(float value, IntVec2 coords)
{
	m_values[coords.x + coords.y * m_dimensions.x] += value;
}

void TileHeatMap::Add(float value, int x, int y)
{
	m_values[x + y * m_dimensions.x] += value;
}

IntVec2 TileHeatMap::CoordsFromIndex(int index)
{
	IntVec2 tileCoords(index % m_dimensions.x, index / m_dimensions.x);
	return tileCoords;
}

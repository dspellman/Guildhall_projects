#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include <vector>
#include "Engine/Math/IntVec2.hpp"

class TileHeatMap
{
public:
	~TileHeatMap() {};
	TileHeatMap() {};
	TileHeatMap(IntVec2 const& dimensions);

	void SetAllValues(float fillValue);
	void FloodFill(int x, int y, float value);
	float Get(IntVec2 coords) const;
	float Get(int x, int y) const;
	void Set(float value, IntVec2 coords);
	void Set(float value, int x, int y);
	void Add(float value, IntVec2 coords);
	void Add(float value, int x, int y);
	std::vector<float> GetData() { return m_values; }
	IntVec2 CoordsFromIndex(int index);

protected:
	IntVec2 m_dimensions;
	std::vector<float> m_values;
};
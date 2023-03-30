#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/FloatRange.hpp"

FloatRange const FloatRange::ZERO = FloatRange(0.0f, 0.0f);
FloatRange const FloatRange::ONE = FloatRange(1.0f, 1.0f);
FloatRange const FloatRange::ZERO_TO_ONE = FloatRange(0.0f, 1.0f);

FloatRange::~FloatRange()
{
}

FloatRange::FloatRange()
	: m_min(0.0f), m_max(0.0f)
{
}

FloatRange::FloatRange(const FloatRange& copyFrom)
{
	m_min = copyFrom.m_min;
	m_max = copyFrom.m_max;
}

FloatRange::FloatRange(float min, float max)
{
	m_min = min;
	m_max = max;
}

FloatRange::FloatRange(Vec2 range)
{
	m_min = range.x;
	m_max = range.y;
}

bool FloatRange::IsOnRange(float const value) const
{
	if (value < m_min || value > m_max)
	{
		return false;
	}
	return true;
}

bool FloatRange::IsOverlappingWith(FloatRange const range) const
{
	if (IsOnRange(range.m_min) || IsOnRange(range.m_max))
	{
		return true;
	}
	return false;
}

bool FloatRange::operator!=(const FloatRange& compare) const
{
	if (m_min == compare.m_min && m_max == compare.m_max)
	{
		return false;
	}
	return true;
}

bool FloatRange::operator==(const FloatRange& compare) const
{
	if (m_min == compare.m_min && m_max == compare.m_max)
	{
		return true;
	}
	return false;
}

void FloatRange::operator=(const FloatRange& copyFrom)
{
	m_min = copyFrom.m_min;
	m_max = copyFrom.m_max;
}



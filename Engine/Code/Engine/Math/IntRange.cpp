#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/IntRange.hpp"

IntRange const IntRange::ZERO = IntRange(0.0f, 0.0f);
IntRange const IntRange::ONE = IntRange(1.0f, 1.0f);
IntRange const IntRange::ZERO_TO_ONE = IntRange(0.0f, 1.0f);

IntRange::IntRange()
	: m_min(0.0f), m_max(0.0f)
{
}

IntRange::IntRange(const IntRange& copyFrom)
{
	m_min = copyFrom.m_min;
	m_max = copyFrom.m_max;
}

IntRange::IntRange(float min, float max)
{
	m_min = min;
	m_max = max;
}

IntRange::IntRange(Vec2 range)
{
	m_min = range.x;
	m_max = range.y;
}

bool IntRange::IsOnRange(float const value) const
{
	if (value < m_min || value > m_max)
	{
		return false;
	}
	return true;
}

bool IntRange::IsOverlappingWith(IntRange const range) const
{
	if (IsOnRange(range.m_min) || IsOnRange(range.m_max))
	{
		return true;
	}
	return false;
}

bool IntRange::operator!=(const IntRange& compare) const
{
	if (m_min == compare.m_min && m_max == compare.m_max)
	{
		return false;
	}
	return true;
}

bool IntRange::operator==(const IntRange& compare) const
{
	if (m_min == compare.m_min && m_max == compare.m_max)
	{
		return true;
	}
	return false;
}

void IntRange::operator=(const IntRange& copyFrom)
{
	m_min = copyFrom.m_min;
	m_max = copyFrom.m_max;
}



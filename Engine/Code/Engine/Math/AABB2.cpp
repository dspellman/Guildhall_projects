#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Vec2.hpp"

#define _USE_MATH_DEFINES
#include <math.h>
#include <Engine/Core/EngineCommon.hpp>
#include <string>
#include <vector>


AABB2 const AABB2::UNIT_BOX = AABB2(Vec2(-0.5f, -0.5f), Vec2(0.5f, 0.5f));
AABB2 const AABB2::ZERO_TO_ONE = AABB2(Vec2(0.0f, 0.0f), Vec2(1.0f, 1.0f));

AABB2::AABB2(float minx, float miny, float maxx, float maxy)
{
	m_mins.x = minx;
	m_mins.y = miny;
	m_maxs.x = maxx;
	m_maxs.y = maxy;
}

AABB2::AABB2(Vec2 const& mins, Vec2 const& maxs)
{
	m_mins = mins;
	m_maxs = maxs;
}

AABB2::AABB2(AABB2 const& copyFrom)
{
	m_mins = copyFrom.m_mins;
	m_maxs = copyFrom.m_maxs;
}

AABB2::AABB2()
{

}

AABB2::~AABB2()
{

}

bool AABB2::IsPointInside(Vec2 const point) const
{
	if (point.x < m_maxs.x && point.x > m_mins.x && point.y < m_maxs.y && point.y > m_mins.y)
		return true;
	return false;
}

Vec2 AABB2::GetCenter() const
{
	return Vec2((m_mins.x + m_maxs.x) / 2.0f, (m_mins.y + m_maxs.y) / 2.0f);
}

Vec2 AABB2::GetDimensions() const
{
	return Vec2(m_maxs.x - m_mins.x, m_maxs.y - m_mins.y);
}

Vec2 AABB2::GetNearestPoint(Vec2 const& referencePoint) const
{
	return Vec2(GetClamped(referencePoint.x, m_mins.x, m_maxs.x), GetClamped(referencePoint.y, m_mins.y, m_maxs.y));
}

Vec2 AABB2::GetPointAtUV(Vec2 const& uv) const
{
	return Vec2(RangeMap(uv.x, 0.0f, 1.0f, m_mins.x, m_maxs.x), RangeMap(uv.y, 0.0f, 1.0f, m_mins.y, m_maxs.y));
}

Vec2 AABB2::GetUVForPoint(Vec2 const& point) const
{
	return Vec2(RangeMap(point.x, m_mins.x, m_maxs.x, 0.0f, 1.0f), RangeMap(point.y, m_mins.y, m_maxs.y, 0.0f, 1.0f));
}

std::string AABB2::ToString()
{
	std::string content = "(";
	content += m_mins.ToString();
	content += ",";
	content += m_maxs.ToString();
	content += ")";
	return content;
}

void AABB2::Translate(Vec2 const& translation)
{
	m_mins += translation;
	m_maxs += translation;
}

void AABB2::SetCenter(Vec2 const& newCenter)
{
	Vec2 direction = newCenter - GetCenter();
	m_mins += direction;
	m_maxs += direction;
}

void AABB2::SetDimensions(Vec2 const& newDimensions)
{
	Vec2 center = GetCenter();
	m_mins.x = center.x - newDimensions.x / 2.0f;
	m_mins.y = center.y - newDimensions.y / 2.0f;
	m_maxs.x = center.x + newDimensions.x / 2.0f;
	m_maxs.y = center.y + newDimensions.y / 2.0f;
}

void AABB2::StretchToIncludePoint(Vec2 const& point)
{
	if (m_maxs.x < point.x)
	{
		m_maxs.x = point.x;
	}
	if (m_mins.x > point.x)
	{
		m_mins.x = point.x;
	}
	if (m_maxs.y < point.y)
	{					 
		m_maxs.y = point.y;
	}					 
	if (m_mins.y > point.y)
	{					 
		m_mins.y = point.y;
	}
}

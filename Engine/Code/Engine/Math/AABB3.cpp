#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Vec3.hpp"

#define _USE_MATH_DEFINES
#include <math.h>
#include <Engine/Core/EngineCommon.hpp>
#include <string>
#include <vector>


AABB3 const AABB3::UNIT_BOX = AABB3(Vec3(-0.5f, -0.5f, -0.5f), Vec3(0.5f, 0.5f, 0.5f));
AABB3 const AABB3::ZERO_TO_ONE = AABB3(Vec3(0.0f, 0.0f, 0.0f), Vec3(1.0f, 1.0f, 1.0f));

AABB3::AABB3(float minx, float miny, float minz, float maxx, float maxy, float maxz)
{
	m_mins.x = minx;
	m_mins.y = miny;
	m_mins.z = minz;
	m_maxs.x = maxx;
	m_maxs.y = maxy;
	m_maxs.z = maxz;
}

AABB3::AABB3(Vec3 const& mins, Vec3 const& maxs)
{
	m_mins = mins;
	m_maxs = maxs;
}

AABB3::AABB3(AABB3 const& copyFrom)
{
	m_mins = copyFrom.m_mins;
	m_maxs = copyFrom.m_maxs;
}

AABB3::AABB3()
{

}

AABB3::~AABB3()
{

}

bool AABB3::IsPointInside(Vec3 const point) const
{
	if (point.x < m_maxs.x && point.x > m_mins.x && point.y < m_maxs.y && point.y > m_mins.y && point.z < m_maxs.z && point.z > m_mins.z)
		return true;
	return false;
}

Vec3 AABB3::GetCenter() const
{
	return Vec3((m_mins.x + m_maxs.x) / 2.0f, (m_mins.y + m_maxs.y) / 2.0f, (m_mins.z + m_maxs.z) / 2.0f);
}

Vec3 AABB3::GetDimensions() const
{
	return Vec3(m_maxs.x - m_mins.x, m_maxs.y - m_mins.y, m_maxs.z - m_mins.z);
}

Vec3 AABB3::GetNearestPoint(Vec3 const& referencePoint) const
{
	return Vec3(GetClamped(referencePoint.x, m_mins.x, m_maxs.x), GetClamped(referencePoint.y, m_mins.y, m_maxs.y), GetClamped(referencePoint.z, m_mins.z, m_maxs.z));
}

// This function is wrong probably
Vec2 AABB3::GetPointAtUV(Vec2 const& uv) const
{
	return Vec2(RangeMap(uv.x, 0.0f, 1.0f, m_mins.x, m_maxs.x), RangeMap(uv.y, 0.0f, 1.0f, m_mins.y, m_maxs.y));
}

// This function is wrong probably
Vec2 AABB3::GetUVForPoint(Vec2 const& point) const
{
	return Vec2(RangeMap(point.x, m_mins.x, m_maxs.x, 0.0f, 1.0f), RangeMap(point.y, m_mins.y, m_maxs.y, 0.0f, 1.0f));
}

std::string AABB3::ToString()
{
	std::string content = "(";
// 	content += m_mins.ToString();
// 	content += ",";
// 	content += m_maxs.ToString();
	content += ")";
	return content;
}

void AABB3::Translate(Vec3 const& translation)
{
	m_mins += translation;
	m_maxs += translation;
}

void AABB3::SetCenter(Vec3 const& newCenter)
{
	Vec3 direction = newCenter - GetCenter();
	m_mins += direction;
	m_maxs += direction;
}

void AABB3::SetDimensions(Vec3 const& newDimensions)
{
	Vec3 center = GetCenter();
	m_mins.x = center.x - newDimensions.x / 2.0f;
	m_mins.y = center.y - newDimensions.y / 2.0f;
	m_mins.z = center.z - newDimensions.z / 2.0f;
	m_maxs.x = center.x + newDimensions.x / 2.0f;
	m_maxs.y = center.y + newDimensions.y / 2.0f;
	m_maxs.z = center.z + newDimensions.z / 2.0f;
}

void AABB3::StretchToIncludePoint(Vec3 const& point)
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

	if (m_maxs.z < point.z)
	{
		m_maxs.z = point.z;
	}
	if (m_mins.z > point.z)
	{
		m_mins.z = point.z;
	}
}

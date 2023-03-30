#include "Engine\Math\LineSegment2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "RaycastUtils.hpp"

LineSegment2::~LineSegment2()
{

}

LineSegment2::LineSegment2()
	: m_startPos(Vec2::ZERO), m_endPos(Vec2::ZERO)
{

}

LineSegment2::LineSegment2(const Vec2 startPos, Vec2 const endPos)
	: m_startPos(startPos), m_endPos(endPos)
{

}

Vec2 LineSegment2::GetNearestPoint(Vec2 const& point) const
{
	Vec2 startToEnd = m_endPos - m_startPos;
	Vec2 endToPoint = point - m_endPos;
	Vec2 startToPoint = point - m_startPos;

	if (DotProduct2D(startToEnd, endToPoint) >= 0.0f)
	{
		return m_endPos;
	}
	if (DotProduct2D(startToEnd, startToPoint) <= 0.0f)
	{
		return m_startPos;
	}
	return m_startPos + GetProjectedOnto2D(startToPoint, startToEnd); // projected onto line segment
}

RaycastResult2D LineSegment2::TestRaycast(Vec2 startPos, Vec2 fwdNormal, float maxDist)
{
	return RaycastVsLineSegment2D(startPos, fwdNormal, maxDist, m_startPos, m_endPos);
}


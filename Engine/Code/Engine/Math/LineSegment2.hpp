#pragma once
#include "Engine\Math\Vec2.hpp"

struct RaycastResult2D;

struct LineSegment2
{
public:
	Vec2 m_startPos = Vec2::ZERO;
	Vec2 m_endPos = Vec2::ZERO;

	~LineSegment2();
	LineSegment2();
	explicit LineSegment2(const Vec2 startPos, Vec2 const endPos);

	Vec2 GetNearestPoint(Vec2 const& point) const;
	RaycastResult2D TestRaycast(Vec2 startPos, Vec2 fwdNormal, float maxDist);
};
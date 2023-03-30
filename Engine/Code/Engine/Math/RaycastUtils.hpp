#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "AABB2.hpp"
#include "AABB3.hpp"
#include "LineSegment2.hpp"
#include "OBB2.hpp"

struct RaycastResult2D
{
	// Basic ray cast result information (required)
	bool	m_didImpact = false;
	float	m_impactDist = 0.0f;
	float	m_fraction = 0.0f;
	Vec2	m_impactPos;
	Vec2	m_impactNormal;

	// Original ray cast information (optional)
	Vec2	m_rayFwdNormal;
	Vec2	m_rayStartPos;
	float	m_rayMaxLength = 1.0f;
	IntVec2 tileCoords = IntVec2::ZERO; // optional parameter for tiled map processing
};

struct RaycastResult3D
{
	// Basic ray cast result information (required)
	bool	m_didImpact = false;
	float	m_fraction = 0.0f;
	float	m_impactDist = 0.0f;
	Vec3	m_impactPos;
	Vec3	m_impactNormal;

	// Original ray cast information (optional)
	Vec3	m_rayFwdNormal;
	Vec3	m_rayStartPos;
	float	m_rayMaxLength = 1.0f;
};

RaycastResult2D RaycastVsLineSegment2D(Vec2 startPos, Vec2 fwdNormal, float maxDist, Vec2 lineStart, Vec2 lineEnd);
RaycastResult2D RaycastVsDisc2D(Vec2 startPos, Vec2 fwdNormal, float maxDist, Vec2 discCenter, float discRadius);
RaycastResult2D RaycastVsAABB2D(Vec2 startPos, Vec2 fwdNormal, float maxDist, AABB2 bounds);
RaycastResult2D RaycastVsOBB2D(Vec2 startPos, Vec2 fwdNormal, float maxDist, OBB2 bounds);

RaycastResult3D RaycastVsSphere(Vec3 startPos, Vec3 fwdNormal, float maxDist, Vec3 center, float radius);
RaycastResult3D RaycastVsZCylinder(Vec3 start, Vec3 fwdNormal, float maxDist, Vec3 center, float length, float radius);
RaycastResult3D RaycastVsAABB3D(Vec3 startPos, Vec3 fwdNormal, float maxDist, AABB3 bounds);

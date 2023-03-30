#pragma once
#include "Engine/Math/Vec2.hpp"

struct OBB2
{
public:
	Vec2 m_center;
	Vec2 m_iBasisNormal;
	Vec2 m_halfDimensions;
	float m_boundingRadius = 0.0f;

	~OBB2();
	OBB2();
	OBB2(Vec2 center, Vec2 iBasisNormal, Vec2 halfDimensions);

	void GetCornerPoints( Vec2* out_fourCornerWorldPositions ) const; // for drawing, etc.
	Vec2 GetLocalPosForWorldPos(Vec2 worldPos) const;
	Vec2 GetWorldPosForLocalPos(Vec2 localPos) const;
	void RotateAboutCenter(float rotationDeltaDegrees);
};
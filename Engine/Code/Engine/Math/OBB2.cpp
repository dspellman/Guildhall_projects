#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/OBB2.hpp"

OBB2::~OBB2()
{

}

OBB2::OBB2()
	: m_center(Vec2::ZERO), m_iBasisNormal(Vec2(1.0f, 0.0f)), m_halfDimensions(0.5f, 0.5f)
{
	m_boundingRadius = m_halfDimensions.GetLength();
}

OBB2::OBB2(Vec2 center, Vec2 iBasisNormal, Vec2 halfDimensions)
	: m_center(center), m_iBasisNormal(iBasisNormal), m_halfDimensions(halfDimensions)
{
	m_boundingRadius = m_halfDimensions.GetLength();
}

void OBB2::GetCornerPoints(Vec2* out_fourCornerWorldPositions) const
{
	// Use CCW ordering for texture u,v processing, topRight, topLeft, bottomLeft, bottomRight ordering
	Vec2 xHalfPos = m_iBasisNormal * m_halfDimensions.x;
	Vec2 yHalfPos = m_iBasisNormal.GetRotated90Degrees() * m_halfDimensions.y;

	out_fourCornerWorldPositions[0] = m_center + xHalfPos + yHalfPos;  // topRight
	out_fourCornerWorldPositions[1] = m_center - xHalfPos + yHalfPos;  // topLeft
	out_fourCornerWorldPositions[2] = m_center - xHalfPos - yHalfPos;  // bottomLeft
	out_fourCornerWorldPositions[3] = m_center + xHalfPos - yHalfPos;  // bottomRight
}

Vec2 OBB2::GetLocalPosForWorldPos(Vec2 worldPos) const
{
	Vec2 centerToPoint = worldPos - m_center;
	Vec2 localPos(DotProduct2D(centerToPoint, m_iBasisNormal), DotProduct2D(centerToPoint, m_iBasisNormal.GetRotated90Degrees()));
	return localPos;
}

Vec2 OBB2::GetWorldPosForLocalPos(Vec2 localPos) const
{
	Vec2 worldPos(m_center + m_iBasisNormal * localPos.x + m_iBasisNormal.GetRotated90Degrees() * localPos.y);
	return worldPos;
}

void OBB2::RotateAboutCenter(float rotationDeltaDegrees)
{
	m_iBasisNormal.RotateDegrees(rotationDeltaDegrees);
}

